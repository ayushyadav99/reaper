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
    //void* ptr;
    //char data[1];
};

// struct for freelist with prev, next (freelist) and a pointer to memory_block
struct free_list_node{
    struct free_list_node* prev_node;
    struct free_list_node* next_node;
    struct memory_block* mem_block;
};

#define BLOCK_SIZE sizeof(struct memory_block)
// for 64 bit systems the minimum becomes 8 bytes
#define MIN_ALLOC_SZ BLOCK_SIZE + 8


// TODO: Determine exact number to use of alignment
/* The macro align4 is used to set the requested size to multiple of four greater than requested size */
#define align8(x) (((((x)-1) >> 3) << 3) + 8)

void* list_head=NULL;
void* list_tail=NULL;
void* free_list_head=NULL;

struct memory_block* split_block(struct memory_block* block,size_t size);
struct free_list_node* find_free_block(struct free_list_node** last_block, size_t size);
struct memory_block* request_from_os(struct memory_block* last_block, size_t size);
void free_node_from_free_list(struct free_list_node* node_that_goes);
void* my_malloc(size_t size);
void display_free_list();
void display_mem_map();



struct free_list_node* find_free_node(size_t size){
    struct free_list_node* curr=free_list_head;
    //printf("find free\n");
    struct free_list_node* ans=NULL;
    while(curr!=NULL){
        if(curr->mem_block->free==1 && curr->mem_block->size>=size){
            ans=curr;
            // printf("brooke\n");
            break;
        }
        else{
            printf("no find\n");
            curr=curr->next_node;
        }
    }
    //printf("end_free\n");
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
struct memory_block* request_from_os(struct memory_block* last_block, size_t size){
    printf("requesting from os\n");
    size_t page_size = getpagesize();
    //printf("mmap pre11\b");
    // printf("page size: %lu\n", page_size);
    //printf("mmap pre1\n");
    struct memory_block* new_block=NULL;
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
        //perror("mmap failed\n");
        return NULL;
    }
    //printf("mmap done\n");
    new_block = (struct memory_block*)requested_block;
    // if(last_block!=NULL){
    //     new_block->prev_block=last_block->prev_block;
    //     last_block->prev_block=new_block;
    // }
    new_block->size=n_pages*page_size-BLOCK_SIZE;
    new_block->free=0;
    new_block->next_block=NULL;
    new_block->prev_block=NULL;
    //new_block->ptr=new_block->data;
    struct memory_block* l_head=(struct memory_block*)(list_head);
    struct memory_block* l_tail=(struct memory_block*)(list_tail);
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
    struct memory_block* res=new_block;
    if(new_block->size-tot_size>=MIN_ALLOC_SZ){
        //split_block
        res=split_block(new_block,tot_size);
    }
    //printf("done\n");
    return res;
}

struct memory_block* split_block(struct memory_block* block,size_t size){
    display_mem_map();
    printf("pre_block_split\n");
    printf("block size: %zu\n    size: %zu\n",block->size,size);
    display_free_list();
    struct memory_block* n_block=NULL;
    n_block=(struct memory_block*)(block+BLOCK_SIZE+size); //start address of new block
    n_block->size=block->size-size-BLOCK_SIZE;
    n_block->next_block=block->next_block;
    n_block->prev_block=block;
    n_block->free=1;
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
    add_node_to_free_list_head(n_free_node);
    printf("block_split");
    display_free_list();
    display_mem_map();
    return block;
}

void* my_malloc(size_t size){
    struct memory_block* block=NULL;
    struct free_list_node* free_node=NULL;
    size_t s;
    s=align8(size);
    if(free_list_head!=NULL){
        //printf("free_list_not_empty\n");
        free_node=find_free_node(s);
        if(free_node!=NULL){
            block=free_node->mem_block;
            // printf("%f\n",block->size);
            if(block->size-s>=MIN_ALLOC_SZ){
                //split_block
                printf("call split\n");
                block=split_block(block,s);
                //printf("got split\n");
            }
            block->free=0;
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
        printf("free_list_empty\n");
        block=request_from_os(NULL,s);
        if(block==NULL){
            return NULL;
        }
        //list_head=block;
    }
    //printf("got node\n");
    free_node_from_free_list(free_node);
    //printf("free_node_freelist_done\n");
    printf("block_size\n");
    printf("%zu\n",block->size);
    return block+1;
}

struct memory_block* get_memory_block_ptr(void* ptr){
    printf("get_memptr\n");
    return (struct memory_block*)(ptr-1);
}

struct free_list_node* find_free_list_node(struct memory_block* block)
{
    struct free_list_node* curr = free_list_head;
    while(curr != NULL && curr->mem_block != block)
    {
        curr=curr->next_node;
    }
    return curr;
}

void free_node_from_free_list(struct free_list_node* node_that_goes){
    //printf("about_tofree1\n");
    if(node_that_goes!=NULL){
        //delete free node from free list
        //printf("about_tofree2\n");
        struct free_list_node* prev_free=node_that_goes->prev_node;
        struct free_list_node* next_free=node_that_goes->next_node;
        //printf("about_tofree3\n");
        if(prev_free){
            //printf("about_tofree4\n");
            //printf("%p",next_free->mem_block->size);
            if(next_free){
                //printf("about_tofree7\n");
                prev_free->next_node=next_free;
                //printf("about_tofree8\n");
                next_free->prev_node=prev_free;
                //printf("about_tofree9\n");
            }
            else{
                //printf("about_tofree10\n");
                prev_free->next_node=NULL;
            }
        }
        // else if(next_free){
        //     printf("about_tofree5\n");
        //     next_free->prev_node=NULL;
        // }
        else{
            //printf("about_tofree6\n");
            free_list_head=next_free;
            if(next_free){
                next_free->prev_node=NULL;
            }
        }
        //printf("about_tofree\n");
        free(node_that_goes);
    } 
}

struct memory_block* coalesce_blocks(struct memory_block* block){
    if(block->next_block!=NULL && block->next_block->free==1){
        printf("inside_if\n");
        if(((struct memory_block*)((char*)(block) + block->size))==block->next_block){
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
    printf("myfree\n");
    //display_free_list();
    if(addr_valid(ptr)==1){
        printf("is_addr_valid\n");
        struct memory_block* memory_block_ptr=get_memory_block_ptr(ptr);
        printf("got mem array\n");
        memory_block_ptr->free=1;
        printf("got free\n");
        // if  block is larger than 2 pages then directly release to os
        size_t page_size = getpagesize();
        if(memory_block_ptr->size+BLOCK_SIZE>=2*page_size){
            printf("munmap\n");
            munmap(memory_block_ptr,memory_block_ptr->size+BLOCK_SIZE);
            return;
        }
        // add node that points to block to to head of free list
        //struct free_list_node* free_head=(struct free_list_node*)free_list_head;
        struct free_list_node* n_free_node=(struct free_list_node*)(malloc(sizeof(struct free_list_node)));
        n_free_node->mem_block=memory_block_ptr;
        add_node_to_free_list_head(n_free_node);
        printf("myfree2\n");
        display_free_list();
        if(memory_block_ptr->prev_block!=NULL && memory_block_ptr->prev_block->free==1){
            //merge previous block since its free
            printf("merge 1\n");
            memory_block_ptr=coalesce_blocks(memory_block_ptr->prev_block);
        }

        if(memory_block_ptr->next_block!=NULL){
            // try to merge with next block if possible
            printf("merge 2\n");
            memory_block_ptr=coalesce_blocks(memory_block_ptr);
        }
        printf("myfree3\n");
        //display_free_list();
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
        if(trav->mem_block->free==0)
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
    printf("Size of memory block: %zu\n", sizeof(struct memory_block));
    struct memory_block* trav = list_head;
    while(trav != NULL)
    {
        printf("=================================================\n");
        printf("Allocation Status: ");
        if(trav->free)
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
struct Student {
    int id;
    float gpa;
  };
  
  int main(int argc, char** argv) {
    if (argc != 2) {
      printf("specify number of mallocs\n");
      return 1;
    }
    const int num_objects = atoi(argv[1]);
  
    for (int i = 0; i < num_objects; i++) {
    struct Student* student = (struct Student*)my_malloc(sizeof(struct Student));
  
    if (student == NULL) {
      printf("Memory allocation failed!\n");
      return 1;
    }
      student->id = i + 1;
      student->gpa = 3.0;  
    printf("student: ID = %d, GPA = %.2f\n", student->id, student->gpa);
    my_free(student);
    }
  
  
    return 0;
  }

//   // ./a.out 1000000  0.45s user 0.64s system 26% cpu 4.166 total mmap
//   // ./a.out 1000000  0.43s user 0.64s system 25% cpu 4.111 total sbrk
//   // ./a.out 1000000  0.45s user 0.65s system 26% cpu 4.132 total baseline
//   // ./a.out 1000000  1.07s user 3.19s system 69% cpu 6.115 total ayush's