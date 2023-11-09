#include <iostream>
#include <string.h>
#include <unistd.h>
#define MAX_ALLOCATION_SIZE 100000000


typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
}MetData;

class LIST_ALLOCATION{
public:
    struct MallocMetadata* head;
    int list_size;
    
    ////function
    LIST_ALLOCATION(){
	head=NULL;
    list_size=0;
	}
	
    void insert(MetData * p);
    size_t countFreeBytes();
    size_t countFreeBlocks();
    size_t countAllBytes();
   
};
//////////////////GLOBAL PARAM////////////////////////////////

LIST_ALLOCATION allocation_list=LIST_ALLOCATION();

//////////////////ALLOCATION LIST FUNCTIONS////////////////////

 void insert(MetData * p){

    //EMPTY_LIST
    allocation_list.list_size++;
    if (allocation_list.head == NULL){
        allocation_list.head=p;
        p->prev= NULL;
        p->next= NULL;
        return;
    } else{

		//FIND PLACE TO PUT
		MetData* it=allocation_list.head;
		//not sorted??
		while(it->next!=NULL){
			it=it->next;
		}
		
        p->next = NULL;
        p->prev= it;
        p->prev->next= p;

        return ;
    }

}

size_t LIST_ALLOCATION::countFreeBytes(){
    MetData *it = allocation_list.head;
    size_t counter = 0;
    while(it){
        if(it->is_free==true)
        {
            counter+=it->size;

        }
        it = it->next;
    }
    return counter;
 }
 
 
size_t LIST_ALLOCATION::countAllBytes(){
    MetData *it = allocation_list.head;
    size_t counter = 0;
    while(it){
        counter+=it->size;
        it = it->next;
    }
    return counter;
}


size_t LIST_ALLOCATION::countFreeBlocks(){
    MetData *it = allocation_list.head;
    size_t count_free = 0;
    while(it){
        if(it->is_free==true)
        {
			count_free++;
		}
        it = it->next;
        }
    return count_free;
}


void* smalloc(size_t size){
if(size==0||size>MAX_ALLOCATION_SIZE){
    return NULL;
}
    MetData* it=allocation_list.head;
    size_t size_needed=size+ sizeof(MetData);
    MetData* temp=NULL;
    while (it){
        if(it->size>=size&&it->is_free== true){
			if(temp==NULL||temp>it){
            temp=it;
            }
        }
        it=it->next;
    }
    if(temp){
		temp->is_free= false;
        return (void*)(temp+1);
	}
    //not finded enough
    void* my_new_alloc=sbrk(size_needed);
    if(my_new_alloc==(void*)-1){
        return NULL;
    }
    MetData* my_new_all=(MetData*)my_new_alloc;
    my_new_all->is_free= false;
    my_new_all->size=size;
    
    insert((MetData*)my_new_alloc);
    //no metadata
    return ( void*)(my_new_all+1);
}

void* scalloc(size_t num, size_t size)
{
    ///FAILURE
    if(size==0 || num ==0 || size*num>MAX_ALLOCATION_SIZE)
    {
        return NULL;
    }
    void * free_block = smalloc(num*size);
    if(free_block==NULL)
    {
        return NULL;
    }
    ///memset sets the first count bytes of dest to the value
    memset(free_block,0,num*size);
    ///SUCCESS
    return free_block;

}

void sfree(void* p)
{
    if(p==NULL)
    {
        return;}
    MetData* DIE=((MetData*)(p))-1;
    //GET METADATA
   // DIE--;
    DIE->is_free=true;
}

void* srealloc(void* oldp, size_t size)
{
    if(size==0 || size> MAX_ALLOCATION_SIZE)
    {
        return NULL;
    }
/* If ‘oldp’ is NULL, allocates space for ‘size’ bytes and returns a pointer to it.*/
    if(oldp==NULL)
    {
        return smalloc(size);
    }

    /*If ‘size’ is smaller than or equal to the current block’s size, reuses the same block*/

    MetData * my_old_b =(MetData*)(oldp)-1;
    //my_old_b--;
    if(size <= my_old_b->size) {
        return oldp;
    }

    /*finds/allocates ‘size’ bytes for a new space, copies content of oldp into the
new allocated space and frees the oldp*/

    void* new_space = smalloc(size);
    if(new_space == NULL) {
        return NULL;//failure
    }
//succes
    /* copies n characters from memory area src to memory area dest.*/
    memmove(new_space, oldp, my_old_b->size);
    sfree(oldp);
    return new_space;

}

size_t _num_free_blocks()
{
  return allocation_list.countFreeBlocks();
}

size_t _num_free_bytes()
{
    return allocation_list.countFreeBytes();


}

size_t _num_allocated_blocks()
{
    return allocation_list.list_size;
}


size_t _num_allocated_bytes()
{
    return allocation_list.countAllBytes();

}

size_t _num_meta_data_bytes()
{
	return allocation_list.list_size*sizeof(MetData);

}

size_t _size_meta_data()
{
    return sizeof(MetData);
}
