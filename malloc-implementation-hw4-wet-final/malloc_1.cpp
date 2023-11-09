//
// Created by student on 1/15/23.
//
#include <unistd.h>
#define MAX 100000000

/* in case of Succeess returns the pointer */
/*sbrk returns -1 if an error occured*/
void* smalloc(size_t size){
    void* allocated;
    if(size == 0 || size > MAX) {
        return NULL;
    }
    allocated = sbrk(size);

    if(allocated == (void*)-1) {
        return NULL;
    }
    return allocated;
}
