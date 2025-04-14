// this should be more optimised as it uses a doubly linked list and also has mechanisms to join and split blocks of memory
// !!Need to test for correctness!!

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include<math.h>

// block header size is 48 bytes currently
struct memory_block{
    int free;
    size_t size;
    struct memory_block* next_block;
    struct memory_block* prev_block;
    void* ptr;
    char data[1];
};

// struct for freelist with prev, next (freelist) and a pointer to memory_block

#define BLOCK_SIZE sizeof(struct memory_block)
// for 64 bit systems the minimum becomes 8 bytes
#define MIN_ALLOC_SZ BLOCK_SIZE + 8


// TODO: Determine exact number to use of alignment
/* The macro align4 is used to set the requested size to multiple of four greater than requested size */
#define align8(x) (((((x)-1) >> 3) << 3) + 8)

void* list_head=NULL;

struct memory_block* split_block(struct memory_block* block,size_t size);
struct memory_block* find_free_block(struct memory_block** last_block, size_t size);
struct memory_block* request_from_os(struct memory_block* last_block, size_t size);
void* my_malloc(size_t size);

struct memory_block* syscall_for_mem(size_t size) {
    struct memory_block* new_block = NULL;
#if defined(__APPLE__) || defined(__MACH__)
    new_block = mmap(NULL, 
                 size,
                 PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANON,
                 -1,
                 0);
    if (new_block == MAP_FAILED) {
      return NULL;
    }
#elif defined(__linux__)
    new_block=sbrk(0);
    void* requested_block=sbrk(size);
    assert((void*)new_block == requested_block); // not thread safe
    if(requested_block == (void*)-1){
        return NULL;
    }
#endif
    return new_block;
}

void os_memory_return(struct memory_block* ptr, size_t size){
#if defined(__APPLE__) || defined(__MACH__)
  munmap(ptr, size);
#elif defined(__linux__)
  brk(ptr);
#endif
}


struct memory_block* find_free_block(struct memory_block** last_block, size_t size){
    struct memory_block* curr=list_head;
    while(curr!=NULL && !(curr->free==true && curr->size>=size)){
        *last_block = curr;
        curr=curr->next_block;
    }
    return *last_block;
}

// request 2 pages from os not sbrk
struct memory_block* request_from_os(struct memory_block* last_block, size_t size){
    printf("requesting from os\n");
    size_t page_size = getpagesize();
    printf("page size: %lu\n", page_size);
    struct memory_block* new_block;
    size_t tot_size=size+BLOCK_SIZE;
    size_t n_pages;
    if(tot_size%page_size!=0){
        n_pages=fmax(2,tot_size/page_size+1);
    }
    else{
        n_pages=fmax(2,tot_size/page_size);
    }
    void* requested_block = syscall_for_mem(n_pages*page_size);
    new_block = (struct memory_block*)requested_block;
    if(last_block!=NULL){
        new_block->prev_block=last_block->prev_block;
        last_block->prev_block=new_block;
    }
    new_block->size=n_pages*page_size-BLOCK_SIZE;
    new_block->free=false;
    new_block->next_block=NULL;
    new_block->prev_block=NULL;
    new_block->ptr=new_block->data;
    if(last_block){
        last_block->next_block=new_block;
        new_block->prev_block=last_block;
    }
    struct memory_block* res=new_block;
    if(new_block->size-tot_size>=MIN_ALLOC_SZ){
        //split_block
        res=split_block(new_block,tot_size);
    }
    return res;
}

struct memory_block* split_block(struct memory_block* block,size_t size){
    struct memory_block* n_block;
    //n_block=(struct memory_block*)(block->data+size); //start address of new block
    n_block=(struct memory_block*)(block->data+block->size-size-BLOCK_SIZE);
    //n_block->size=block->size-size-BLOCK_SIZE;
    n_block->size=size;
    n_block->next_block=block->next_block;
    n_block->prev_block=block;
    n_block->free=0;
    block->free=1;
    n_block->ptr=n_block->data;
    block->next_block=n_block;
    block->size=size;
    if(n_block->next_block!=NULL){
        n_block->next_block->prev_block=n_block;
    }
    return n_block;
}

void* my_malloc(size_t size){
    struct memory_block* block;
    struct memory_block* last_block;
    size_t s;
    s=align8(size);
    if(list_head){
        last_block=list_head;
        block=find_free_block(&last_block,s);
        if(block){
            if(block->size-s>=MIN_ALLOC_SZ){
                //split_block
                block=split_block(block,s);
            }
            block->free=0;
        }
        else{
            block=request_from_os(NULL,s);
            if(!block){
                return NULL;
            }
        }
    }
    else{
        block=request_from_os(NULL,s);
        if(!block){
            return NULL;
        }
        list_head=block;
    }
    // delete block from free list
    struct memory_block* prev=block->prev_block;
    struct memory_block* next=block->next_block;
    if(prev==NULL && next==NULL){
        list_head=NULL;
    }
    else if(prev!=NULL && next==NULL){
        prev->next_block=NULL;
    }
    else if(prev==NULL && next!=NULL){
        list_head=next;
        next->prev_block=NULL;
    }
    else{
        prev->next_block=next;
        next->prev_block=prev;
    }
    return block->data;
}

struct memory_block* get_memory_block_ptr(void* ptr){
    return (struct memory_block*)ptr-1;
}

struct memory_block* coalesce_blocks(struct memory_block* block){
    if(block->next_block!=NULL && block->next_block->free==1){
        block->size+=BLOCK_SIZE+block->next_block->size;
        block->next_block=block->next_block->next_block;
    }
    if(block->next_block!=NULL){
        block->next_block->prev_block=block;
    }
    return block;
}

int addr_valid(void* p){
    if(list_head){
        if(p>list_head){
            if(p==get_memory_block_ptr(p)->ptr){
                return 1;
            }
        }
    }
    return 0;
}

void my_free(void* ptr){
    if(addr_valid(ptr)==1){
        struct memory_block* memory_block_ptr=get_memory_block_ptr(ptr);
        memory_block_ptr->free=1;
        // if  block is larger than 2 pages then directly release to os
        size_t page_size = getpagesize();
        if(memory_block_ptr->size+BLOCK_SIZE>=2*page_size){
            os_memory_return(memory_block_ptr, memory_block_ptr->size+BLOCK_SIZE);
            return;
        }
        struct memory_block* prev=memory_block_ptr->prev_block;
        struct memory_block* curr=list_head;
        if(prev==NULL){
            if(curr){
                memory_block_ptr->next_block=curr;
                curr->prev_block=memory_block_ptr;
                list_head=memory_block_ptr;
            }
            else{
                memory_block_ptr->prev_block=NULL;
                memory_block_ptr->next_block=NULL;
                list_head=memory_block_ptr;
            }
        }
        else{
            while(curr!=prev && curr->next_block){
                curr=curr->next_block;
            }
            //insert after curr
            if(curr->next_block){
                memory_block_ptr->prev_block=curr;
                memory_block_ptr->next_block=curr->next_block;
                curr->next_block->prev_block=memory_block_ptr;
                curr->next_block=memory_block_ptr;
            }
            else{
                curr->next_block=memory_block_ptr;
                memory_block_ptr->prev_block=curr;
            }
        }
        
        if(memory_block_ptr->prev_block!=NULL && memory_block_ptr->prev_block->free==1){
            //merge previous block since its free
            memory_block_ptr=coalesce_blocks(memory_block_ptr->prev_block);
        }
        if(memory_block_ptr->next_block!=NULL){
            // try to merge with next block if possible
            memory_block_ptr=coalesce_blocks(memory_block_ptr);
        }
        else{
            if(memory_block_ptr->prev_block!=NULL){
                memory_block_ptr->prev_block->next_block=NULL;
            }
            else{
                list_head=NULL;
            }
            os_memory_return(memory_block_ptr, sizeof(struct memory_block)+BLOCK_SIZE);
        }
    }
    return;
}


// realloc needs tweaks
void* realloc(void* ptr, size_t size){
    if(ptr==NULL){
        return malloc(size);
    }

    if(addr_valid(ptr)==1){
        size_t s=align8(size);
        struct memory_block* memory_block_ptr=get_memory_block_ptr(ptr);
        if(memory_block_ptr->size>=s){
            if(memory_block_ptr->size>=MIN_ALLOC_SZ){
                split_block(memory_block_ptr,s);
            }
        }
        else{
            if(memory_block_ptr->next_block && memory_block_ptr->next_block->free==1 && ((memory_block_ptr->size+memory_block_ptr->next_block->size+BLOCK_SIZE)>=s)){
                memory_block_ptr=coalesce_blocks(memory_block_ptr);
                if(memory_block_ptr->size-s>=MIN_ALLOC_SZ){
                    split_block(memory_block_ptr,s);
                }
            }
            else{
                //malloc a new block
                void* new_ptr=malloc(s);
                if(new_ptr==NULL){
                    return NULL;
                }
                struct memory_block* n_block=get_memory_block_ptr(new_ptr);
                memcpy(n_block->data,memory_block_ptr->data,s);
                free(ptr);
                return n_block;
            }
        }
        return ptr;
    }
    return NULL;
}

void *calloc(size_t nelem, size_t elsize) {
    size_t size = nelem * elsize;
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}

