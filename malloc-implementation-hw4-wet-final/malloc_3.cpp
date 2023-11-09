


#include <iostream>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#define MAX_ALLOCATION_SIZE 100000000
#define USE_MMAP (128*1024)
#define cookie_values 10000
//#define  cookie_global 376094

//////////////////GLOBAL PARAM////////////////////////////////

int  cookie_global= rand() % cookie_values;
class LIST_ALLOCATION;
 struct MallocMetadata {
private:
    int Cookie;
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
public:
    MallocMetadata(){
        Cookie=cookie_global;
        size=0;
        is_free= true;
        next=NULL;
        prev=NULL;
    }
    ~MallocMetadata()=default;

    size_t get_mmd_size(){

        if(this->Cookie!=cookie_global)
        {
            exit(0xdeadbeef);
        }
        return this->size;
    }

    void set_mmd_is_free(bool Is_free){

        if(this->Cookie!=cookie_global)
        {
            exit(0xdeadbeef);
        }

        this->is_free=Is_free;
        return;
    }

    bool get_mmd_is_free(){

        if(this->Cookie!=cookie_global)
        {
            exit(0xdeadbeef);
        }
        return this->is_free;
    }
    void set_mmd_cookie(int cookie){
        this->Cookie=cookie;
        return;
    }



    MallocMetadata* get_mmd_prev()
    {

        if(this->Cookie!=cookie_global) {
            exit(0xdeadbeef);
        }
        return this->prev;
    }

    MallocMetadata* get_mmd_next()
    {

        if(this->Cookie!=cookie_global) {
            exit(0xdeadbeef);
        }
        return this->next;
    }

    void set_mmd_size(size_t MY_size)
    {

        if(this->Cookie!=cookie_global)
        {
            exit(0xdeadbeef);
        }

        this->size=MY_size;
        return;
    }
    void set_mmd_prev(MallocMetadata* MY_prev)
    {

        if(this->Cookie!=cookie_global)
        {
            exit(0xdeadbeef);
        }

        this->prev=MY_prev;
        return;
    }
    void set_mmd_next(MallocMetadata* MY_next)
    {

        if(this->Cookie!=cookie_global)
        {
            exit(0xdeadbeef);
        }

        this->next=MY_next;
        return;
    }

    friend LIST_ALLOCATION;
};

void split(MallocMetadata *to_split,size_t full_size,size_t taken_size);
bool which_to_choose ( MallocMetadata* key1,  MallocMetadata* key2){
    // if(! key1|| !key2){return;}
    if (key1->get_mmd_size() != key2->get_mmd_size()){
        return (key1->get_mmd_size() <key2->get_mmd_size());
    }
    return (key1<key2);
}






class LIST_ALLOCATION{
public:
    size_t mmap_bl;
    size_t mmap_bytes;
    MallocMetadata* head;
    int list_size;
    MallocMetadata* biggest_addres;

    ////function
    LIST_ALLOCATION(){
        //cookie_global= rand() % cookie_values;
        mmap_bl=0;
        mmap_bytes=0;
        head=NULL;
        list_size=0;
        biggest_addres=NULL;

        }





    void* _called_by_smalloc(size_t size){

        if(size==0||size>MAX_ALLOCATION_SIZE){
            return NULL;
        }
        MallocMetadata* it=head;
        size_t size_needed=size+ sizeof(MallocMetadata);
        MallocMetadata* temp=NULL;
        while (it!=NULL){
            // size and then adres
            if(it->get_mmd_size()>=size&&it->get_mmd_is_free()== true){
                if(temp==NULL|| which_to_choose(it,temp)){
                    temp=it;
                }
            }
            it=it->get_mmd_next();
        }
        if(temp){
            if(128+sizeof(MallocMetadata) <= (temp->get_mmd_size() -(size))){
                MallocMetadata* to_keep=temp;
                split(to_keep, temp->get_mmd_size()+sizeof(MallocMetadata),size_needed);
            }
            // temp->size=size;
            temp->set_mmd_is_free(false);
            return (void*)(temp+1);
        }
        //not finded enough
        if(size>=USE_MMAP){

            void* my_new_alloc=mmap(NULL,size_needed,PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
            if(my_new_alloc==MAP_FAILED){
                return NULL;
            }
            MallocMetadata* my_new_all=(MallocMetadata*)my_new_alloc;
            my_new_all->set_mmd_cookie(cookie_global);
            my_new_all->set_mmd_is_free( false);
            my_new_all->set_mmd_size(size);
            mmap_bytes+=size;
            mmap_bl+=1;
            //no metadata
            return (void*)(my_new_all+1);
        }

        if(biggest_addres&&biggest_addres->get_mmd_is_free() == true){
            //enlarge and finish
            size_needed=size_needed-(biggest_addres->get_mmd_size()+sizeof(MallocMetadata));
            sbrk(size_needed);
            biggest_addres->set_mmd_is_free(false);
            biggest_addres->set_mmd_size(size);
            return (void*)(biggest_addres+1);
        }
        void* my_new_alloc=sbrk(size_needed);
        if(my_new_alloc==(void*)-1){
            return NULL;
        }
        MallocMetadata* my_new_all=(MallocMetadata*)my_new_alloc;
        my_new_all->set_mmd_cookie( cookie_global);
        my_new_all->set_mmd_is_free( false);
        my_new_all->set_mmd_size(size);
        if(biggest_addres == NULL || biggest_addres < my_new_all){
            biggest_addres=my_new_all;
        }

        insert((MallocMetadata*)my_new_alloc);
        //no metadata
        //size_t r=sizeof (MetData);
        return ( void*)(my_new_all+1);
    }

    void remove_from_list(MallocMetadata * p){

        if (p == NULL||list_size==0){
            return ;
        }

        if (list_size==1){
            LIST_ALLOCATION::list_size--;

            head= NULL;
            return ;
        }
        LIST_ALLOCATION::list_size--;

        MallocMetadata * M_prev = p->get_mmd_prev();
        MallocMetadata * M_next = p->get_mmd_next();;

        if(M_prev){
            M_prev->set_mmd_next(M_next);
        }
        if(M_next){
            M_next->set_mmd_prev(M_prev);  }

    }
    void called_by_sfree(void* p)
    {
        if(p==NULL){
            return;}

        MallocMetadata* DIE=(MallocMetadata*)(p)-1;

        if (DIE->get_mmd_size()>=USE_MMAP){
            mmap_bytes-=(DIE->get_mmd_size());
            mmap_bl-=1;
            void* M=(void*)DIE;
            int RET=munmap(M,(size_t)DIE->get_mmd_size()+sizeof(MallocMetadata));
            if (RET==-1){
                return;
            }

            return;
        }
        if(DIE->get_mmd_next()&& DIE->get_mmd_next()->get_mmd_is_free() && DIE->get_mmd_prev()&& DIE->get_mmd_prev()->get_mmd_is_free()){
            if(biggest_addres==DIE->next){
                biggest_addres=DIE->prev;
            }
            DIE->prev->is_free=true;
            DIE->get_mmd_prev()->set_mmd_size(DIE->get_mmd_prev()->get_mmd_size()+DIE->get_mmd_next()->get_mmd_size()+DIE->get_mmd_size()+2*sizeof(MallocMetadata));
            LIST_ALLOCATION::remove_from_list(DIE->get_mmd_next());
            LIST_ALLOCATION::remove_from_list(DIE);
            return;

        }

        if(DIE->get_mmd_next()&&DIE->get_mmd_next()->get_mmd_is_free()){
            if(biggest_addres==DIE->get_mmd_next()){
                biggest_addres=DIE;
            }
            DIE->set_mmd_is_free(true);
            DIE->set_mmd_size(DIE->get_mmd_next()->get_mmd_size()+DIE->get_mmd_size()+sizeof(MallocMetadata));
            LIST_ALLOCATION::remove_from_list( DIE->get_mmd_next());
            return;
        }if(DIE->get_mmd_prev()&&DIE->get_mmd_prev()->get_mmd_is_free()){
            if(biggest_addres==DIE){
                biggest_addres=DIE->get_mmd_prev();
            }
            DIE->get_mmd_prev()->set_mmd_is_free(true);
            DIE->get_mmd_prev()->set_mmd_size(DIE->get_mmd_prev()->get_mmd_size()+DIE->get_mmd_size()+sizeof(MallocMetadata));
            LIST_ALLOCATION::remove_from_list(DIE);
            return;
        }
        DIE->set_mmd_is_free(true);
    }

    void insert(MallocMetadata * p){

        if(!p){
            return;}
        p->set_mmd_next(NULL);
        p->set_mmd_prev(NULL);


        MallocMetadata* it=head;
        while( it!=NULL && it!=p){
            it=it->get_mmd_next();
        }

        if(it==p){
            return;
        }

        //EMPTY_LIST
        list_size++;

        if (head == NULL){
            head=p;
            head->set_mmd_prev(NULL);
            head->set_mmd_next(NULL) ;
            return;
        } else{

            //FIND PLACE TO PUT
            it=head;
            while( it!=NULL && p > it){
                it=it->get_mmd_next();
            }

            if(it==NULL){
//is last
                MallocMetadata* my=head;
                while( my->get_mmd_next()!=NULL){
                    my=my->next;
                }
                my->set_mmd_next(p);
                p->set_mmd_prev(my);
                p->set_mmd_next(NULL);
                return;
                //put after

            }


            MallocMetadata *MY_PREV=it->get_mmd_prev();
            MallocMetadata *MY_NEXT=it->get_mmd_next();
            p->set_mmd_next(it);
            if(MY_NEXT){
                MY_NEXT->set_mmd_prev(p);
            }
            if(MY_PREV){
                MY_PREV->set_mmd_next(p);
            }
            p->set_mmd_prev(MY_PREV);

            ///put beffor it:done

            return ;
        }
    }
    void* called_by_scalloc(size_t num, size_t size)
    {
        ///FAILURE
        if(size==0 || num ==0 || size*num>MAX_ALLOCATION_SIZE)
        {
            return NULL;
        }
        void * free_block = _called_by_smalloc(num*size);
        if(free_block==NULL)
        {
            return NULL;
        }
        ///memset sets the first count bytes of dest to the value
        memset(free_block,0,num*size);
        ///SUCCESS
        return free_block;

    }
    size_t countFreeBytes(){
        MallocMetadata *it = head;
        size_t counter = 0;
        while(it){
            if(it->get_mmd_is_free()==true)
            {
                counter+=it->get_mmd_size();

            }
            it= it->get_mmd_next();
        }
        return counter;
    }
    size_t countFreeBlocks(){
        MallocMetadata *it = head;
        size_t count_free = 0;
        while(it){
            if(it->get_mmd_is_free()==true)
            {
                count_free++;
            }
            it = it->get_mmd_next();
        }
        return count_free;
    }
    size_t countAllBytes(){
        MallocMetadata *it = head;
        size_t counter = 0;
        while(it){
            counter+=it->get_mmd_size();
            it = it->get_mmd_next();
        }
        return counter;
    }
    ~LIST_ALLOCATION(){
        head= nullptr;
        list_size=0;
    }

    void* called_by_srealoc(void* oldp, size_t size) {

        if (size == 0 || size > MAX_ALLOCATION_SIZE) {
            return NULL;
        }
// If ‘oldp’ is NULL, allocates space for ‘size’ bytes and returns a pointer to it./
        if (oldp == NULL) {
            return _called_by_smalloc(size);
        }
        //If size is smaller than or equal to the current block’s size, reuses the same block/
        MallocMetadata *my_old_b = (MallocMetadata *) (oldp) - 1;
        //my_old_b--;
        if (size == my_old_b->get_mmd_size()) {
            my_old_b->set_mmd_is_free (false);
            return oldp;
        }

        if (my_old_b->get_mmd_size() < USE_MMAP) {
            if (size <= my_old_b->get_mmd_size()) {
                my_old_b->set_mmd_is_free(false);

                if (128 + sizeof(MallocMetadata) <= (my_old_b->get_mmd_size() - size)) {
                    MallocMetadata *to_keep = my_old_b;
                    split(to_keep, my_old_b->get_mmd_size() + sizeof(MallocMetadata), sizeof(MallocMetadata) + size);
                }
                return oldp;
            } else if (my_old_b->get_mmd_prev() && my_old_b->get_mmd_prev()->get_mmd_is_free()) {
                if (my_old_b->get_mmd_size() + my_old_b->get_mmd_prev()->get_mmd_size()+ sizeof(MallocMetadata) >= size) {
                    my_old_b->get_mmd_prev()->set_mmd_size(my_old_b->get_mmd_prev()->get_mmd_size()+my_old_b->get_mmd_size() + sizeof(MallocMetadata));
                    my_old_b->get_mmd_prev()->set_mmd_is_free(false);
                    MallocMetadata *TO_ONE = my_old_b->get_mmd_prev();
                    if (my_old_b == biggest_addres) {//AND FIT
                        biggest_addres = TO_ONE;
                    }
                    LIST_ALLOCATION::remove_from_list(my_old_b);
//check split
                    if (128 + sizeof(MallocMetadata) <= (TO_ONE->get_mmd_size() - size)) {
                        MallocMetadata *to_keep = TO_ONE;

                        split(to_keep, TO_ONE->get_mmd_size() + sizeof(MallocMetadata), sizeof(MallocMetadata) + size);
                    }
                    memmove((void*)((my_old_b->get_mmd_prev())+1), oldp, my_old_b->get_mmd_size());
                    return (void *) ((TO_ONE) + 1);
                }
                if (my_old_b == biggest_addres && size > my_old_b->get_mmd_size()+ my_old_b->get_mmd_prev()->get_mmd_size() + sizeof(MallocMetadata)) {
                    // if(size-(my_old_b->size+my_old_b->prev->size+sizeof(MetData))>USE_MMAP){
                    //  }else{
                    sbrk(size - (my_old_b->get_mmd_size() + my_old_b->get_mmd_prev()->get_mmd_size() + sizeof(MallocMetadata)));

                    //   }
                    my_old_b->get_mmd_prev()->set_mmd_size(size);
                    my_old_b->get_mmd_prev()->set_mmd_is_free(false);
                    biggest_addres = my_old_b->get_mmd_prev();
                    MallocMetadata * yu=my_old_b->get_mmd_prev();
                    LIST_ALLOCATION::remove_from_list(my_old_b);
                    memmove((void*)((my_old_b->get_mmd_prev())+1), oldp, my_old_b->get_mmd_size());
                    return (void *) ((yu) + 1);
                }
            }

            if (my_old_b == biggest_addres) {
                sbrk(size - (my_old_b->get_mmd_size()));
                my_old_b->set_mmd_size(size);
                my_old_b->set_mmd_is_free(false);
                return (void *) ((biggest_addres) + 1);
            }//c. If the block is the wilderness chunk, enlarge it.
            if (my_old_b->get_mmd_next() && my_old_b->get_mmd_next()->get_mmd_is_free()) {
                if (my_old_b->get_mmd_size() + sizeof(MallocMetadata) + my_old_b->get_mmd_next()->get_mmd_size()>= size) {
                    size_t old_size=my_old_b->get_mmd_size();
                    my_old_b->set_mmd_size(old_size+my_old_b->get_mmd_next()->get_mmd_size()+sizeof(MallocMetadata));
                    my_old_b->set_mmd_is_free(false);
                    MallocMetadata *Y = my_old_b;
                    if (my_old_b->get_mmd_next() == biggest_addres) {//AND FIT
                        biggest_addres = my_old_b;
                    }
                    LIST_ALLOCATION::remove_from_list(my_old_b->get_mmd_next());
                    if (128 + sizeof(MallocMetadata) <= (Y->get_mmd_size() - size)) {
                        MallocMetadata *to_keep = Y;
                        split(to_keep,Y->get_mmd_size()+sizeof (MallocMetadata), sizeof(MallocMetadata)+size);
                    }

                    return (void *) ((Y) + 1);

                }
            }//Try to merge with the adjacent block with the higher address.
            if (my_old_b->get_mmd_prev() && my_old_b->get_mmd_prev()->get_mmd_is_free() && my_old_b->get_mmd_next() && my_old_b->get_mmd_next()->get_mmd_is_free()) {
                if (my_old_b->get_mmd_size() + my_old_b->get_mmd_prev()->get_mmd_size() + my_old_b->get_mmd_next()->get_mmd_size() + 2 * sizeof(MallocMetadata) >= size) {
                    size_t size_to=my_old_b->get_mmd_prev()->get_mmd_size();
                    my_old_b->get_mmd_prev()->set_mmd_size(size_to+my_old_b->get_mmd_size()+my_old_b->get_mmd_next()->get_mmd_size()+2 * sizeof(MallocMetadata));
                    my_old_b->get_mmd_prev()->set_mmd_is_free(false);
                    if (my_old_b->get_mmd_next()== biggest_addres) {//AND FIT
                        biggest_addres = my_old_b->get_mmd_prev();
                    }
                    MallocMetadata *u = my_old_b->get_mmd_prev();
                    LIST_ALLOCATION::remove_from_list(my_old_b);
                    LIST_ALLOCATION::remove_from_list(my_old_b->get_mmd_next());
                    if (128 + sizeof(MallocMetadata) <= (u->get_mmd_size() - size)) {
                        MallocMetadata *to_keep = u;
                        split(to_keep,u->get_mmd_size()+sizeof (MallocMetadata), sizeof(MallocMetadata)+size);
                    }
                    memmove((void*)((my_old_b->get_mmd_prev())+1), oldp, my_old_b->get_mmd_size());
                    return (void *) ((u) + 1);
                }
                if (my_old_b->get_mmd_next() == biggest_addres &&
                    size > my_old_b->get_mmd_size() + my_old_b->get_mmd_prev()->get_mmd_size()+ my_old_b->get_mmd_next()->get_mmd_size() + 2 * sizeof(MallocMetadata)) {

                    sbrk(size - (my_old_b->get_mmd_size() + my_old_b->get_mmd_prev()->get_mmd_size() + my_old_b->get_mmd_next()->get_mmd_size() + 2 * sizeof(MallocMetadata)));

                    my_old_b->get_mmd_prev()->set_mmd_size(size);
                    my_old_b->get_mmd_prev()->set_mmd_is_free(false);
                    biggest_addres = my_old_b->get_mmd_prev();
                    LIST_ALLOCATION::remove_from_list(my_old_b->get_mmd_next());
                    LIST_ALLOCATION::remove_from_list(my_old_b);
                    memmove((void*)((my_old_b->get_mmd_prev())+1), oldp, my_old_b->get_mmd_size());

                    return (void *) ((my_old_b->get_mmd_prev()) + 1);
                }  //If the wilderness chunk is the adjacent block with the higher address:
                // Try to merge with the lower and upper blocks (such as in e), and enlarge the wilderness block as needed.

            }//Try to merge all those three adjacent blocks together.
            if (my_old_b->get_mmd_next() && my_old_b->get_mmd_next()->get_mmd_is_free() && biggest_addres == my_old_b->get_mmd_next()) {
                if (my_old_b->get_mmd_size() + my_old_b->get_mmd_next()->get_mmd_size()+ sizeof(MallocMetadata) < size) {
                    sbrk(size - (my_old_b->get_mmd_size() + my_old_b->get_mmd_next()->get_mmd_size() + sizeof(MallocMetadata)));
                    my_old_b->set_mmd_size(size);
                    my_old_b->set_mmd_is_free(false);
                    biggest_addres = my_old_b;
                    remove_from_list(my_old_b->get_mmd_next());
                    return (void *) ((my_old_b) + 1);

                }
            }
            // sfree(oldp);
            MallocMetadata *it = head;
            // size_t size_needed=size+sizeof(MetData);
            MallocMetadata *temp =NULL;
            while (it) {
                // size and then adres
                if (it->get_mmd_size() >= size && it->get_mmd_is_free() == true) {
                    if (temp == NULL || which_to_choose(it, temp)) {
                        temp = it;
                    }
                }
                it = it->get_mmd_next();
            }//Try to find a different block that’s large enough to contain the request
            if (temp) {
                called_by_sfree(oldp);
                temp->set_mmd_is_free(false);
                if (128 + sizeof(MallocMetadata) <= (temp->get_mmd_size() - size)) {
                    MallocMetadata *to_keep = temp;
                    split(to_keep,temp->get_mmd_size()+sizeof (MallocMetadata), sizeof(MallocMetadata)+size);
                }
                memmove((void*)(temp+1), oldp, my_old_b->get_mmd_size());
                return (void *) (temp + 1);
            }
        }///GERE>>>???
        size_t size_needed = size + sizeof(MallocMetadata);
        void *new_space = sbrk(size_needed);
        if (new_space == NULL) {
            return NULL;//failure
        }
        MallocMetadata *new_space_meta = (MallocMetadata *) new_space;
        //finds/allocates ‘size’ bytes for a new space, copies content of oldp into the new allocated space and frees the oldp/
        //void* new_space = smalloc(size);
        new_space_meta->set_mmd_cookie(cookie_global);
        new_space_meta->set_mmd_is_free(false);
        new_space_meta->set_mmd_size(size);
        LIST_ALLOCATION::insert(new_space_meta);
        if (biggest_addres == NULL || new_space_meta > biggest_addres) {
            biggest_addres = new_space_meta;
        }
        //succes
        //copies n characters from memory area src to memory area dest.
        memmove((void*)(new_space_meta+1), oldp, my_old_b->get_mmd_size());
        my_old_b->set_mmd_is_free(true);
        return (void*)(new_space_meta+1);
    }
};
//////////////////ALLOCATION LIST FUNCTIONS////////////////////
LIST_ALLOCATION allocation_list=LIST_ALLOCATION();
//full size==+meta data
void split(MallocMetadata *to_split,size_t full_size,size_t taken_size){
    to_split->set_mmd_size(taken_size-sizeof(MallocMetadata));
    //to_split->size=taken_size-sizeof(MallocMetadata);
    to_split->set_mmd_is_free(false);
    //  to_split->is_free=false;
//remove_from_list(to_split);
    // size_t x=taken_size;
    //    MetData* TO_FREE=( MetData*)(to_split+(x));
    // unsigned char * ONE_AFTER= (unsigned char*)to_split + x;
    MallocMetadata * TO_FREE= (MallocMetadata*) ((unsigned char *)to_split +taken_size);
   
    //TO_FREE->is_free=true;
    TO_FREE->set_mmd_cookie(cookie_global);
     TO_FREE->set_mmd_is_free(true);
    TO_FREE->set_mmd_size(full_size-taken_size-sizeof(MallocMetadata));
    //TO_FREE->size=full_size-taken_size-sizeof(MallocMetadata);

    if(allocation_list.biggest_addres==NULL||TO_FREE > allocation_list.biggest_addres){
        allocation_list.biggest_addres=TO_FREE;
    }
    // sfree((void*)(TO_FREE+1));
    //  insert(to_split);
    allocation_list.insert(TO_FREE);
}
void sfree(void* p)
{
    return allocation_list.called_by_sfree(p);
}
///////////////// malloc function //////////////////////
void* smalloc(size_t size){
    return allocation_list._called_by_smalloc(size);
}
void* scalloc(size_t num, size_t size)
{return allocation_list.called_by_scalloc(num,size);
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
    return allocation_list.mmap_bl+allocation_list.list_size;
}
size_t _num_allocated_bytes()
{
    return allocation_list.mmap_bytes+allocation_list.countAllBytes();
}
size_t _num_meta_data_bytes()
{
    return sizeof(MallocMetadata)*allocation_list.mmap_bl+allocation_list.list_size*sizeof(MallocMetadata);
}
size_t _size_meta_data()
{
    return sizeof(MallocMetadata);
}
void* srealloc(void* oldp, size_t size)
{
    return allocation_list.called_by_srealoc(oldp,size);
}
