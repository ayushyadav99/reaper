#include <stdio.h>
#include <omp.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>

void *global_super_block_pointer[64] = {NULL};
pthread_mutex_t lock[64] = {PTHREAD_MUTEX_INITIALIZER};

struct super_block_meta {
    int owner_thread_id;
    size_t mapped_size;
    struct super_block_meta* next;
    bool directly_mapped;
};

struct block_meta {
    size_t size;
    struct block_meta* next;
    struct block_meta* prev;
    struct super_block_meta* my_super_block;
    bool free;
};

#define SUPER_BLOCK_META_SIZE (sizeof(struct super_block_meta))
#define BLOCK_META_SIZE (sizeof(struct block_meta))
#define SUPER_BLOCK_SIZE ((64*getpagesize()) - BLOCK_META_SIZE - SUPER_BLOCK_META_SIZE)
#define MINIMUM_BLOCK_SIZE 8

struct block_meta* find_block_in_super_block(struct super_block_meta* super_block, size_t size) {
    struct block_meta* current = (struct block_meta*) (super_block + 1);
    while(current && !(current->free == true && current->size >= size )) {
        current = current->next;
    }
    return current;
}

struct super_block_meta *get_new_super_block(struct super_block_meta* last, int thread_id, bool directly_mapped, size_t size) {
    struct super_block_meta* new_super_block = mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);

    if(new_super_block == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if(last) last->next = new_super_block;
    else global_super_block_pointer[thread_id] = new_super_block;

    new_super_block->owner_thread_id = thread_id;
    new_super_block->next = NULL;
    new_super_block->mapped_size = size;
    new_super_block->directly_mapped = directly_mapped;

    struct block_meta* block = (struct block_meta*) (new_super_block + 1);
    block->prev = NULL;
    block->next = NULL;
    block->size = SUPER_BLOCK_SIZE;
    block->free = true;
    block->my_super_block = new_super_block;

    return new_super_block;
}

struct block_meta* get_block_from_super_block(size_t size, int thread_id){
    struct super_block_meta* current = global_super_block_pointer[thread_id];
    struct super_block_meta* last = NULL;

    struct block_meta* block = NULL;

    while(current) {
        if(thread_id == current->owner_thread_id) {
            block = find_block_in_super_block(current, size);
            if(block) {
                break;
            }
        }

        last = current;
        current = current->next;
    }

    if(!current) {
        assert(block == NULL);
        current = get_new_super_block(last, thread_id, false, 64*getpagesize());
        block = find_block_in_super_block(current, size);
    }

    return block;
};

struct block_meta* get_directly_mapped_block(size_t size, int thread_id) {
    struct super_block_meta* new_block = get_new_super_block(NULL, thread_id, true, size);
    struct block_meta* block = (struct block_meta*) (new_block+1);

    return block;
}

struct block_meta* split(struct block_meta* block, size_t size) {
    if(block->size - size >= BLOCK_META_SIZE + MINIMUM_BLOCK_SIZE) {
        char* offset_start = (char*)(block +1);
        struct block_meta* new_block = (struct block_meta*)(offset_start+size);

        new_block->size = block->size - BLOCK_META_SIZE - size;
        block->size = size;

        new_block->next = block->next;
        block->next = new_block;
        if(new_block->next) new_block->next->prev = new_block;
        new_block->prev = block;

        new_block->free = block->free;
        new_block->my_super_block = block->my_super_block;
    }
    return block;
}

void* my_malloc(size_t size) {
    size = (size + 7) & ~((size_t)7);
    int thread_id = omp_get_thread_num();
    pthread_mutex_lock(&lock[thread_id]);

    if(size > SUPER_BLOCK_SIZE) {
        struct block_meta *block = get_directly_mapped_block(size,thread_id);

        pthread_mutex_unlock(&lock[thread_id]);
        return (block+1);
    }else if (size <= 0) {

        pthread_mutex_unlock(&lock[thread_id]);
        return NULL;
    }

    struct block_meta *block = get_block_from_super_block(size,thread_id);
    if(!block) {
        pthread_mutex_unlock(&lock[thread_id]);
        return NULL;
    }else {
        block = split(block, size);
        block->free = false;
    }

    pthread_mutex_unlock(&lock[thread_id]);
    return (block+1);
}

void merge (struct block_meta* block) {
    if(block->next) {
        struct block_meta* next = block->next;
        if(next->free) {
            block->size += next->size + BLOCK_META_SIZE;
            block->next = next->next;
            if(next->next) {
                next->next->prev = block;
            }
        }
    }
    if(block->prev) {
        struct block_meta* prev = block->prev;
        if(prev->free) {
            prev->size += block->size + BLOCK_META_SIZE;
            prev->next = block->next;
            if(block->next) {
                block->next->prev = prev;
            }
        }
    }
}

void my_free(void *ptr) {
    if(!ptr) return;
    struct block_meta* block = ((struct block_meta*)ptr) -1;

    pthread_mutex_lock(&lock[block->my_super_block->owner_thread_id]);
    assert(block->free == false);

    if(block->my_super_block->directly_mapped) {
        struct super_block_meta* super_block = ((struct super_block_meta*)ptr) -1;
        munmap(super_block,super_block->mapped_size);
    }else {
        block->free = true;
        merge(block);
    }
    pthread_mutex_unlock(&lock[block->my_super_block->owner_thread_id]);
}

/*
CFLAGS = -Xpreprocessor -fopenmp \
         -I/opt/homebrew/opt/libomp/include \
         -L/opt/homebrew/opt/libomp/lib \
         -lomp -lm -Wall -Iinclude
*/
#define NUM_THREADS 8


int main()
{
    int nthreads = NUM_THREADS;
    int iterations = 1000000;
    double start_time, end_time;

    omp_set_num_threads(nthreads);

    start_time = omp_get_wtime();
    volatile int* abhi;

#pragma omp parallel for
    for(int i = 0; i < iterations; i++) {
        abhi = (int*)my_malloc(32);
    }


    end_time = omp_get_wtime();

    printf("Time elapsed = %f seconds.\n", end_time - start_time);

    return 0;
}