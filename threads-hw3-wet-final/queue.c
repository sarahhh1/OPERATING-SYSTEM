#include "queue.h"
////////////////////////queue c

Queue* create_queue(int max_size){
    Queue* my_new_queue = (Queue*)malloc(sizeof(Queue));
    my_new_queue->size=0;
    my_new_queue->head=NULL;
    my_new_queue->tail=NULL;
    my_new_queue->max_size=max_size;
    return my_new_queue;
}
QueueNODE* node_create(struct timeval arrival,int fd){
    QueueNODE *node=(QueueNODE*) malloc(sizeof(QueueNODE));
    node->prev=NULL;
    node->next=NULL;
    node->arrival=arrival;
    node->fd=fd;

    return node;
}
QueueNODE *insert(Queue* queue, int fd, struct timeval arrival_time){
    if(!queue){
        NULL;
    }
    queue->size++;
    QueueNODE *MY_NODE=node_create(arrival_time,fd);
    if (queue->tail == NULL){
        queue->tail = MY_NODE;
        queue->head=MY_NODE;
        MY_NODE->prev= NULL;
        MY_NODE->next= NULL;
        return MY_NODE;
    }
    if(queue->tail == queue->head){
        queue->tail->next=MY_NODE;
        MY_NODE->next = NULL;
        MY_NODE->prev= queue->tail;
        queue->head= MY_NODE;
        queue->head->prev=MY_NODE->prev;
        queue->head->next= NULL;
       return MY_NODE ;
    }

    MY_NODE->next = NULL;
    MY_NODE->prev= queue->head;
    MY_NODE->prev->next= MY_NODE;
    queue->head = MY_NODE; // not sure
    queue->head->next= NULL;
    return MY_NODE ;
}
///we dont delete it
QueueNODE* get_tail_and_remove(Queue* queue){
    if(!queue){
        return NULL;
    }
    QueueNODE* to_ret=queue->tail;

    if (queue->size==0){
        return NULL;
    }
    if(queue->size==1){
        queue->tail=NULL;
        queue->head=NULL;
        queue->size--;
        return to_ret;
    }
    QueueNODE * temp=queue->tail->next;
    queue->tail->next->prev=NULL;
    queue->tail=temp;
    queue->size--;
    return to_ret;
}
void removeNodeFromQ(Queue *QUEUE,QueueNODE * ode){
    if (ode == NULL){
        return ;
    }
    QUEUE->size--;
    if (ode == QUEUE->tail&& ode==QUEUE->head){
        if(  QUEUE->tail){
            QUEUE->tail->next= NULL;
            QUEUE->tail->prev= NULL;
        }
        if( QUEUE->head)
        {
            QUEUE->head->next= NULL;
            QUEUE->head->prev= NULL;
        }
        if(ode){
            free( ode);
        }
        return ;
    }
    if(ode== QUEUE->tail){
        if (QUEUE->tail != NULL){
            QUEUE->tail = QUEUE->tail->next;
            QUEUE->tail->prev = NULL;
        }

        free(ode);
        return;
    }
    if (ode == QUEUE->head){
        if (QUEUE->head != NULL){
            QUEUE->head = QUEUE->head->prev;
            QUEUE->head->next = NULL;
        }

        free(ode);
        return;
    }
    else{
        QueueNODE * M_prev = ode->prev;
        QueueNODE * M_next = ode->next;
        M_prev->next = M_next;
        M_next->prev = M_prev;
        free(ode);
    }

    return ;
}
QueueNODE* findNodeInQ(Queue *QUEUE,int key){
    QueueNODE * iter=QUEUE->tail;
    while (iter&& iter->fd!=key){
        iter=iter->next;
    }
    return iter;
}


QueueNODE* findNodeByIdx(Queue *QUEUE,int index)
{   int idx=0;
    QueueNODE * iter=QUEUE->tail;
    while (iter){
        if(index==idx)
        {
            return iter;
        }
        iter=iter->next;
        idx++;
    }
    return iter;
}

