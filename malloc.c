//absolutely naive implementation that evolved into a list

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

struct memory_block{
    size_t size;
    struct memory_block* next_block;
    bool free;
};

#define BLOCK_SIZE sizeof(struct memory_block)
void* list_head=NULL;
struct memory_block* find_free_block(struct memory_block** last_block, size_t size){
    struct memory_block* curr=list_head;
    while(curr!=NULL && curr->free==true && curr->size>=size){
        *last_block = curr;
        curr=curr->next_block;
    }
    return curr;
}

struct memory_block* request_from_os(struct memory_block* last_block, size_t size){
    struct memory_block* new_block;
    new_block=sbrk(0);
    void* requested_block=sbrk(size+BLOCK_SIZE);
    assert((void*)new_block == requested_block); // not thread safe
    if(requested_block == (void*)-1){
        return NULL;
    }
    if(last_block!=NULL){
        last_block->next_block=new_block;
    }
    new_block->size=size;
    new_block->free=false;
    new_block->next_block=NULL;
    return new_block;
}

void* malloc(size_t size){
    struct memory_block* block;

    if(size<=0){
        return NULL;
    }
    if(list_head==NULL){
        block=request_from_os(NULL,size);
        if(!block){
            return NULL;
        }
        list_head=block;
    }
    else{
        struct memory_block* last=list_head;
        block=find_free_block(&last, size);
        if(block==NULL){
            block=request_from_os(last,size);
            if(!block){
                return NULL;
            }
        }
        else{
            block->free=false;
        }
    }
    return block+1;
}

struct memory_block* get_memory_block_ptr(void* ptr){
    return (struct memory_block*)ptr-1;
}
void free(void* ptr){
    if(ptr==NULL){
        return;
    }
    struct memory_block* memory_block_ptr=get_memory_block_ptr(ptr);
    assert(memory_block_ptr->free==false);
    memory_block_ptr->free=true;
    return;
}

void* realloc(void* ptr, size_t size){
    if(ptr==NULL){
        return malloc(size);
    }
    struct memory_block* memory_block_ptr=get_memory_block_ptr(ptr);
    if(memory_block_ptr->size>=size){
        return ptr;
    }
    void* new_ptr;
    new_ptr=malloc(size);
    if(new_ptr==NULL){
        return NULL;
    }
    memcpy(new_ptr,ptr,memory_block_ptr->size);
    free(ptr);
    return new_ptr;
}

void *calloc(size_t nelem, size_t elsize) {
    size_t size = nelem * elsize;
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
  }