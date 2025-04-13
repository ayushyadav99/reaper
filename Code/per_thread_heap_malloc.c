#include <stdio.h>
#include <omp.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>

void *global_super_block_pointer = NULL;

struct super_block_meta {
    int owner_thread_id;
    struct super_block_meta* next;
};

struct block_meta {
    size_t size;
    struct block_meta* next;
    struct block_meta* prev;
    bool free;
    bool directly_mapped;
};

#define SUPER_BLOCK_META_SIZE (sizeof(struct super_block_meta))
#define BLOCK_META_SIZE (sizeof(struct block_meta))
#define SUPER_BLOCK_SIZE ((2*getpagesize()) - BLOCK_META_SIZE - SUPER_BLOCK_META_SIZE)
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

    new_super_block->owner_thread_id = thread_id;
    new_super_block->next = NULL;

    struct block_meta* block = (struct block_meta*) (new_super_block + 1);
    block->prev = NULL;
    block->next = NULL;
    block->size = SUPER_BLOCK_SIZE;
    block->free = true;
    block->directly_mapped = directly_mapped;

    return new_super_block;
}

struct block_meta* get_block_from_super_block(size_t size){
    int thread_id = omp_get_thread_num();

    struct super_block_meta* current = global_super_block_pointer;
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
        current = get_new_super_block(last, thread_id, false, 2*getpagesize());
        block = find_block_in_super_block(current, size);
    }

    return block;
};

struct block_meta* get_directly_mapped_block(size_t size) {
    int thread_id = omp_get_thread_num();
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
    }
    return block;
}

void* my_malloc(size_t size) {
    size = (size + 7) & ~((size_t)7);
    if(size > SUPER_BLOCK_SIZE) {
        struct block_meta *block = get_directly_mapped_block(size);
        return (block+1);
    }else if (size <= 0) {
        return NULL;
    }

    struct block_meta *block = get_block_from_super_block(size);
    if(!block) {
        return NULL;
    }else {
        block = split(block, size);
        block->free = false;
    }

return (block+1);
}



int main() {

    int* arr = (int*) my_malloc(5*sizeof(int));
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    arr[4] = 32;

    printf("%lu\n", sizeof(struct super_block_meta));
    printf("%lu\n", sizeof(struct block_meta));

    return 0;
}