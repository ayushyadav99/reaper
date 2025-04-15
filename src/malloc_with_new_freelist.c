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
struct block_meta{
    size_t size;
    struct block_meta* next_block;
    struct block_meta* prev_block;
    struct free_list_node* free_list_ptr;
    void* ptr; //pointer to super block
    bool free;
};

// struct for freelist with prev, next (freelist) and a pointer to memory_block
struct free_list_node{
    struct free_list_node* prev_node;
    struct free_list_node* next_node;
    struct block_meta* mem_block;
};

#define BLOCK_SIZE sizeof(struct block_meta)
// for 64 bit systems the minimum becomes 8 bytes
#define MIN_ALLOC_SZ BLOCK_SIZE + 8


// TODO: Determine exact number to use of alignment
/* The macro align4 is used to set the requested size to multiple of four greater than requested size */
#define align8(x) (((((x)-1) >> 3) << 3) + 8)

void* list_head=NULL;
void* list_tail=NULL;
void* free_list_head=NULL;

struct block_meta* split_block(struct block_meta* block,size_t size);
struct free_list_node* find_free_block(struct free_list_node** last_block, size_t size);
struct block_meta* request_from_os(struct block_meta* last_block, size_t size);
void free_node_from_free_list(struct free_list_node* node_that_goes);
void* my_malloc(size_t size);
void display_free_list();
void display_mem_map();



struct free_list_node* find_free_node(size_t size){
    struct free_list_node* curr=free_list_head;
    struct free_list_node* ans=NULL;
    while(curr!=NULL){
        if(curr->mem_block->free==true && curr->mem_block->size>=size){
            ans=curr;
            break;
        }
        else{
            //printf("no find\n");
            curr=curr->next_node;
        }
    }
    return ans;
}

struct free_list_node* add_node_to_free_list_head(struct free_list_node* f_node){
    //struct free_list_node* free_head=free_list_head;
    f_node->prev_node=NULL;
    f_node->next_node=free_list_head;
    if(f_node->next_node!=NULL){
        f_node->next_node->prev_node=f_node;
    }
    free_list_head=f_node;
    return f_node;
}

// request 2 pages from os not sbrk
struct block_meta* request_from_os(struct block_meta* last_block, size_t size){
<<<<<<< HEAD
    printf("requesting from os\n");
=======
    //printf("requesting from os\n");
>>>>>>> origin
    size_t page_size = getpagesize();
    struct block_meta* new_block=NULL;
    size_t tot_size=size+BLOCK_SIZE;
    size_t n_pages;
    if(tot_size%page_size!=0){
        n_pages=fmax(2,tot_size/page_size+1);
    }
    else{
        n_pages=fmax(2,tot_size/page_size);
    }
    //printf("mmap pre\n");
    void* requested_block = mmap(NULL, n_pages*page_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0);
    if (requested_block == MAP_FAILED) {
        return NULL;
    }
    new_block = (struct block_meta*)requested_block;
    new_block->size=n_pages*page_size-BLOCK_SIZE;
    new_block->free=false;
    new_block->next_block=NULL;
    new_block->prev_block=NULL;
    new_block->ptr=NULL;
    struct block_meta* l_head=(struct block_meta*)(list_head);
    struct block_meta* l_tail=(struct block_meta*)(list_tail);
    if(list_head==NULL){
        list_head=new_block;
        list_tail=list_head;
    }
    else{
        l_head->next_block=new_block;
        l_head->next_block->prev_block=l_head;
        l_tail=new_block;
        list_head=l_head;
        list_tail=l_tail;
    }
    // if(last_block){
    //     last_block->next_block=new_block;
    //     new_block->prev_block=last_block;
    // }
    struct block_meta* res=new_block;
    if(new_block->size-tot_size>=MIN_ALLOC_SZ){
        //split_block
        res=split_block(new_block,tot_size);
    }
    //printf("done\n");
    return res;
}

struct block_meta* split_block(struct block_meta* block,size_t size){
    struct block_meta* n_block=NULL;
    n_block=(struct block_meta*)((char*)(block+1)+size); //start address of new block
    n_block->size=block->size-size-BLOCK_SIZE;
    n_block->next_block=block->next_block;
    n_block->prev_block=block;
    n_block->free=true;
    n_block->ptr=NULL;
    //n_block->ptr=n_block->data;
    block->next_block=n_block;
    block->size=size;
    if(n_block->next_block!=NULL){
        n_block->next_block->prev_block=n_block;
    }
    if(block==list_tail){
        list_tail=n_block;
    }
    // add node that points to block to to head of free list
    struct free_list_node* n_free_node=(struct free_list_node*)(malloc(sizeof(struct free_list_node)));
    n_free_node->mem_block=n_block;
    n_free_node->mem_block->free_list_ptr=n_free_node;
    add_node_to_free_list_head(n_free_node);
    return block;
}

void* my_malloc(size_t size){
    struct block_meta* block=NULL;
    struct free_list_node* free_node=NULL;
    size_t s;
    s=align8(size);
    if(free_list_head!=NULL){
        //printf("free_list_not_empty\n");
        free_node=find_free_node(s);
        if(free_node!=NULL){
            block=free_node->mem_block;
            if(block->size-s>=MIN_ALLOC_SZ){
                //split_block
                block=split_block(block,s);
            }
            block->free=false;
        }
        else{
            block=request_from_os(NULL,s);
            if(block==NULL){
                return NULL;
            }
        }
    }
    else{
        // free list is empty
        block=request_from_os(NULL,s);
        if(block==NULL){
            return NULL;
        }
        //list_head=block;
    }
    free_node_from_free_list(free_node);
    block->free_list_ptr=NULL;
    return block+1;
}

struct block_meta* get_memory_block_ptr(void* ptr){
    return (struct block_meta*)(ptr)-1;
}

struct free_list_node* find_free_list_node(struct block_meta* block)
{
    // struct free_list_node* curr = free_list_head;
    // while(curr != NULL && curr->mem_block != block)
    // {
    //     curr=curr->next_node;
    // }
    // return curr;
    return block->free_list_ptr;
}

void free_node_from_free_list(struct free_list_node* node_that_goes){
    if(node_that_goes!=NULL){
        //delete free node from free list
        struct free_list_node* prev_free=node_that_goes->prev_node;
        struct free_list_node* next_free=node_that_goes->next_node;
        if(prev_free){
            if(next_free){
                prev_free->next_node=next_free;
                next_free->prev_node=prev_free;
            }
            else{
                prev_free->next_node=NULL;
            }
        }
        else{
            free_list_head=next_free;
            if(next_free){
                next_free->prev_node=NULL;
            }
        }
        free(node_that_goes);
    } 
}

struct block_meta* coalesce_blocks(struct block_meta* block){
    if(block->next_block!=NULL && block->next_block->free==true){
        if(((struct block_meta*)((char*)(block+1) + block->size))==block->next_block){
            struct free_list_node* node_that_goes=find_free_list_node(block->next_block);
            block->size=block->size+block->next_block->size+BLOCK_SIZE;
            block->next_block=block->next_block->next_block;
            if(block->next_block!=NULL){
                block->next_block->prev_block=block;
            }
            else{
                list_tail=block;
            }
            //delete node_that_goes
            free_node_from_free_list(node_that_goes); 
        }
    }
    return block;
}

int addr_valid(void* p){
    // if(list_head){
    //     if(p>list_head){
    //         if(p==get_memory_block_ptr(p)->ptr){
    //             return 1;
    //         }
    //     }
    // }
    return 1;
}

void my_free(void* ptr){
    if(addr_valid(ptr)==1){
        struct block_meta* memory_block_ptr=get_memory_block_ptr(ptr);
        memory_block_ptr->free=true;
        // if  block is larger than 2 pages then directly release to os
        size_t page_size = getpagesize();
        if(memory_block_ptr->size+BLOCK_SIZE>=2*page_size){
            //printf("munmap\n");
            munmap(memory_block_ptr,memory_block_ptr->size+BLOCK_SIZE);
            return;
        }
        // add node that points to block to to head of free list
        struct free_list_node* n_free_node=(struct free_list_node*)(malloc(sizeof(struct free_list_node)));
        n_free_node->mem_block=memory_block_ptr;
        n_free_node->mem_block->free_list_ptr=n_free_node;
        add_node_to_free_list_head(n_free_node);
        if(memory_block_ptr->prev_block!=NULL && memory_block_ptr->prev_block->free==true){
            //merge previous block since its free
            memory_block_ptr=coalesce_blocks(memory_block_ptr->prev_block);
        }
        if(memory_block_ptr->next_block!=NULL){
            // try to merge with next block if possible
            memory_block_ptr=coalesce_blocks(memory_block_ptr);
        }
        //if size is still greater than twice page size then release back to os and delete the free list node
        // if(memory_block_ptr->size+BLOCK_SIZE>=2*page_size){
        //     struct free_list_node* node_that_goes=find_free_list_node(memory_block_ptr);
        //     if(memory_block_ptr->prev_block){
        //         memory_block_ptr->prev_block->next_block=memory_block_ptr->next_block;
        //         if(memory_block_ptr->next_block){
        //             memory_block_ptr->next_block->prev_block=memory_block_ptr->prev_block;
        //         }
        //     }
        //     else if(memory_block_ptr->next_block){
        //         memory_block_ptr->next_block->prev_block=NULL;
        //         list_head=memory_block_ptr->next_block;
        //     }
        //     else{
        //         list_head=NULL;
        //     }
        //     free_node_from_free_list(node_that_goes);
        //     munmap(memory_block_ptr,memory_block_ptr->size+BLOCK_SIZE);
        //     return;
        // }
    }
    return;
}

void display_free_list()
{
    printf("\nFree List: \n");
    struct free_list_node* trav = free_list_head;
    while(trav != NULL)
    {
        printf("-------------------------------------------------\n");
        printf("Allocation Status: ");
        if(trav->mem_block->free==false)
        {
            printf("Allocated");
        }
        else
        {
            printf("Free");
        }
        printf("\n");
        printf("Size of Mem Blk: %zu\n", trav->mem_block->size);
        printf("Address of Mem Blk: %p\n", (void*)trav->mem_block);
        
        trav = trav->next_node;
    }
    printf("-------------------------------------------------\n");
}

void display_mem_map()
{
    printf("\nMemory Mapping: \n");
    printf("Size of memory block: %zu\n", sizeof(struct block_meta));
    struct block_meta* trav = list_head;
    while(trav != NULL)
    {
        printf("=================================================\n");
        printf("Allocation Status: ");
        if(trav->free==false)
        {
            printf("Allocated");
        }
        else
        {
            printf("Free");
        }
        printf("\n");
        printf("Size of Mem Blk: %zu\n", trav->size);
        printf("Address of Mem Blk: %p\n", (void*)trav);
        trav = trav->next_block;
    }
    printf("=================================================\n");
}
//TEST CODE BELOW
// struct Student {
//     int id;
//     float gpa;
//   };
  
//   int main(int argc, char** argv) {
//     if (argc != 2) {
//       printf("specify number of mallocs\n");
//       return 1;
//     }
//     const int num_objects = atoi(argv[1]);
  
//     for (int i = 0; i < num_objects; i++) {
//     struct Student* student = (struct Student*)my_malloc(sizeof(struct Student));
  
//     if (student == NULL) {
//       printf("Memory allocation failed!\n");
//       return 1;
//     }
//       student->id = i + 1;
//       student->gpa = 3.0;  
//     printf("student: ID = %d, GPA = %.2f\n", student->id, student->gpa);
//     my_free(student);
//     }
  
//     printf("block size: %zu\n",sizeof(struct block_meta));
//     return 0;
//   }

//   // ./a.out 1000000  0.45s user 0.64s system 26% cpu 4.166 total mmap
//   // ./a.out 1000000  0.43s user 0.64s system 25% cpu 4.111 total sbrk
//   // ./a.out 1000000  0.45s user 0.65s system 26% cpu 4.132 total baseline
//   // ./a.out 1000000  1.07s user 3.19s system 69% cpu 6.115 total ayush's