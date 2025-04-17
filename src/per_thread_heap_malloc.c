#include <stdio.h>
#include <omp.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include<stdlib.h>
#include<stdlib.h>
#include <math.h>

void *global_super_block_pointer[64] = {NULL};
pthread_mutex_t lock[64] = {PTHREAD_MUTEX_INITIALIZER};

struct super_block_meta {
    int owner_thread_id;
    size_t mapped_size;
    size_t available_size;
    struct super_block_meta* next;
    struct block_meta* free_list;
    bool directly_mapped;
};

struct block_meta {
    size_t size;
    struct block_meta* next;
    struct block_meta* prev;
    struct super_block_meta* my_super_block;

    struct block_meta* free_list_next;
    struct block_meta* free_list_prev;
    bool free;
};

#define SUPER_BLOCK_META_SIZE (sizeof(struct super_block_meta))
#define BLOCK_META_SIZE (sizeof(struct block_meta))
#define SUPER_BLOCK_SIZE ((64*getpagesize()) - BLOCK_META_SIZE - SUPER_BLOCK_META_SIZE)
#define MINIMUM_BLOCK_SIZE 8

struct block_meta* find_block_in_super_block(const struct super_block_meta* super_block, size_t size) {
    struct block_meta* current = super_block->free_list;
    while(current && !(current->free == true && current->size >= size )) {
        current = current->free_list_next;
    }
    return current;
}

void add_to_free_list (struct block_meta* block) {
    block->free_list_next = block->my_super_block->free_list;
    block->free_list_prev = NULL;
    block->my_super_block->free_list = block;
    if(block->free_list_next) {
        block->free_list_next->free_list_prev = block;
    }
}

void remove_from_free_list (struct block_meta* block) {
    if(block->free_list_prev) {
        if(block->free_list_next) {
            block->free_list_next->free_list_prev = block->free_list_prev;
            block->free_list_prev->free_list_next = block->free_list_next;
        }else {
            block->free_list_prev->free_list_next = NULL;
        }
    }else {
        if(block->free_list_next) {
            block->free_list_next->free_list_prev = NULL;
        }
        block->my_super_block->free_list = block->free_list_next;
    }

    block->free_list_next = NULL;
    block->free_list_prev = NULL;
}

struct super_block_meta *get_new_super_block(int thread_id, bool directly_mapped, size_t size) {
    struct super_block_meta* new_super_block = mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);

    if(new_super_block == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    if(!directly_mapped) {
        new_super_block->next = global_super_block_pointer[thread_id];
        global_super_block_pointer[thread_id] = new_super_block;
    }else {
        new_super_block->next = NULL;
    }

    new_super_block->owner_thread_id = thread_id;
    new_super_block->next = NULL;
    new_super_block->mapped_size = size;
    new_super_block->directly_mapped = directly_mapped;
    new_super_block->available_size = SUPER_BLOCK_SIZE;
    new_super_block->free_list = NULL;

    struct block_meta* block = (struct block_meta*) (new_super_block + 1);
    block->prev = NULL;
    block->next = NULL;
    block->free_list_next = NULL;
    block->free_list_prev = NULL;
    block->size = SUPER_BLOCK_SIZE;
    block->free = true;
    block->my_super_block = new_super_block;

    add_to_free_list(block);

    return new_super_block;
}

struct block_meta* get_block_from_super_block(size_t size, int thread_id){
    struct super_block_meta* current = global_super_block_pointer[thread_id];

    struct block_meta* block = NULL;

    while(current) {
            if(current->available_size >= size) {
                block = find_block_in_super_block(current, size);
                if(block) {
                    break;
                }
            }

        current = current->next;
    }

    if(!current) {
        assert(block == NULL);
        current = get_new_super_block( thread_id, false, 64*getpagesize());
        block = find_block_in_super_block(current, size);
    }

    return block;
};

struct block_meta* get_directly_mapped_block(size_t size, int thread_id) {
    struct super_block_meta* new_block = get_new_super_block( thread_id, true, size);
    struct block_meta* block = (struct block_meta*) (new_block+1);

    return block;
}

struct block_meta* split(struct block_meta* block, size_t size) {
    if(block->size - size >= BLOCK_META_SIZE + MINIMUM_BLOCK_SIZE) {
        char* offset_start = (char*)(block +1);
        struct block_meta* new_block = (struct block_meta*)(offset_start+size);

        new_block->size = block->size - BLOCK_META_SIZE - size;
        block->my_super_block->available_size -= BLOCK_META_SIZE + size;
        block->size = size;

        new_block->next = block->next;
        block->next = new_block;
        if(new_block->next) new_block->next->prev = new_block;
        new_block->prev = block;

        new_block->free = block->free;
        new_block->my_super_block = block->my_super_block;
        add_to_free_list(new_block);
    }else {
        block->my_super_block->available_size -= block->size;
    }
    remove_from_free_list(block);
    return block;
}

void* my_malloc(size_t size) {
    size = (size + 7) & ~((size_t)7);
    int thread_id = omp_get_thread_num();
    pthread_mutex_lock(&lock[thread_id]);

    if(size > SUPER_BLOCK_SIZE) {
        struct block_meta *block = get_directly_mapped_block(size,thread_id);
        block->free = false;
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
    block->my_super_block->available_size += block->size;
    add_to_free_list(block);
    if(block->next) {
        struct block_meta* next = block->next;
        if(next->free) {
            block->size += next->size + BLOCK_META_SIZE;
            block->my_super_block->available_size += BLOCK_META_SIZE;
            block->next = next->next;
            if(next->next) {
                next->next->prev = block;
            }
            remove_from_free_list(next);
        }
    }
    if(block->prev) {
        struct block_meta* prev = block->prev;
        if(prev->free) {
            remove_from_free_list(block);
            prev->size += block->size + BLOCK_META_SIZE;
            prev->my_super_block->available_size += BLOCK_META_SIZE;
            prev->next = block->next;
            if(block->next) {
                block->next->prev = prev;
            }
            add_to_free_list(prev);
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

