#include "segel.h"
#include "request.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//////////////////////////
pthread_mutex_t mtx;
pthread_cond_t cond;
pthread_cond_t cond_block;

////////////////////////////////////////// my global variable
Queue* waiting_request_queue=NULL;
Queue* handeled_request_queue=NULL;
thread_info * thread_arr=NULL;

void* thread_routine(void* my_id){
    ///true condition
    int* id =(int* )my_id;

    while (1) {

        pthread_mutex_lock(&mtx);
        /// if wait queue is empty
        while (waiting_request_queue->size == 0) {
            pthread_cond_wait(&cond,&mtx);
        }
        ///else
        QueueNODE *to_exec = get_tail_and_remove(waiting_request_queue);
        QueueNODE *to_rem_from_handel=  insert(handeled_request_queue, to_exec->fd, to_exec->arrival);
        int fd_working_now = to_exec->fd;
        free(to_exec);

       // struct timeval to_handel_arrival=to_rem_from_handel->arrival;
      //  struct timeval to_handel_interval=to_rem_from_handel->interval;
        pthread_mutex_unlock(&mtx);

        struct timeval handel_time;
        gettimeofday(&(handel_time), NULL);
        timersub(&handel_time,&(to_rem_from_handel->arrival),&( to_rem_from_handel->interval));
        //fprintf(stderr,"%")
        //pthread_mutex_lock(&mtx);
        requestHandle(fd_working_now,(*id),to_rem_from_handel->arrival,to_rem_from_handel->interval,thread_arr);
       // pthread_mutex_unlock(&mtx);
       // Close(fd_working_now);

        pthread_mutex_lock(&mtx);
        removeNodeFromQ(handeled_request_queue, to_rem_from_handel);
        pthread_cond_signal(&cond_block);
        pthread_mutex_unlock(&mtx);
        Close(fd_working_now);


    }
}


void create_Array_threads(int size){
   thread_arr =(thread_info*)malloc(sizeof(thread_info)*size);
    for (int i=0;i <size;i++){
        thread_arr[i].total_dynamic_requests_handled=0;
        thread_arr[i].total_static_requests_handled=0;
        thread_arr[i].total_requests_handled=0;
    }
}

/////////////////////////////////////// PART 2
void block (int max){
  /*  while (waiting_request_queue->size+handeled_request_queue->size==max){
        pthread_cond_wait(&cond_block, &mtx);
    }*/
}

void drop_tail(int fd){
//    Close(fd);
}

void drop_head(int connfd ){
    /*QueueNODE * TO_DEL=get_tail_and_remove(waiting_request_queue);
    if(TO_DEL){
        Close(TO_DEL->fd);
        free(TO_DEL);
        struct timeval arrival_time;
        gettimeofday(&arrival_time,NULL);
        insert(waiting_request_queue,connfd,arrival_time);
        pthread_cond_signal(&cond);

    }else{
        Close(connfd);
    }*/
}

void drop_random(int connfd){
/*
    if(waiting_request_queue->size==0) {
        Close(connfd);
        return;
    }
    int num_requests_to_drop= (int)((waiting_request_queue->size+1)/2);
    //pthread_mutex_lock(&mtx);
    for(int i = 0; i < num_requests_to_drop; i++) {
            if(waiting_request_queue->size!=0) {
                int random_index = rand() % (waiting_request_queue->size);
                QueueNODE *my_node = findNodeByIdx(waiting_request_queue, random_index);
                removeNodeFromQ(waiting_request_queue, my_node);
                Close(my_node->fd);
                free(my_node);

            }
        }
    insert(waiting_request_queue,connfd);
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&mtx);*/
}

void getargs(int *port, int argc, char *argv[],int *threads_num,int* queue_size,char* schedalg)
{
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    (*port) = atoi(argv[1]);
    (*threads_num)= atoi(argv[2]);
    (*queue_size) = atoi(argv[3]);
    strcpy(schedalg, argv[4]);

}


int main(int argc, char *argv[]) {
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    int threads_num, MAX;
    char schedalg[7] = {0};
    getargs(&port, argc, argv, &threads_num, &MAX, schedalg);
    //////mutex create
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond_block, NULL);
    ////////queue
    waiting_request_queue = create_queue(MAX);
    handeled_request_queue = create_queue(threads_num);
    ////creating threads
  create_Array_threads(threads_num);

    ///////our threads create
    pthread_t *threads = malloc(sizeof(*threads) * threads_num);
    for (int idx = 0; idx < threads_num; idx++) {
        int *id = (int*)malloc(sizeof(int));
        (*id) = idx;
        pthread_create(&threads[idx], NULL, thread_routine, (void *) id);
      //   free(id);
    }

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, (socklen_t *) &clientlen);
        struct timeval arrival_time;
        gettimeofday(&arrival_time, NULL);
int decrease_time=0;
        if (strcmp(schedalg, "block") == 0) {
            decrease_time=1;
        } else if (strcmp(schedalg, "dt") == 0) {
            decrease_time=2;

        } else if (strcmp(schedalg, "dh") == 0) {
            decrease_time=3;


        } else if (strcmp(schedalg, "random") == 0) {
            decrease_time=4;

        }

        pthread_mutex_lock(&mtx);
        if (waiting_request_queue->size + handeled_request_queue->size >= MAX) {
            if (decrease_time==1) {
                while (waiting_request_queue->size + handeled_request_queue->size == MAX) {
                    pthread_cond_wait(&cond_block, &mtx);
                }
                insert(waiting_request_queue, connfd, arrival_time);
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mtx);

            } else if (decrease_time==2) {
                Close(connfd);
                pthread_mutex_unlock(&mtx);
///changed line order

            } else if (decrease_time==3) {
                QueueNODE *TO_DEL = get_tail_and_remove(waiting_request_queue);
                if (TO_DEL) {
                    Close(TO_DEL->fd);
                    free(TO_DEL);
                    insert(waiting_request_queue, connfd, arrival_time);
                    pthread_cond_signal(&cond);
                    pthread_mutex_unlock(&mtx);

                } else {
                    Close(connfd);
                    pthread_mutex_unlock(&mtx);
                }

            } else if (decrease_time==4) {
                if (waiting_request_queue->size == 0) {
                    Close(connfd);
                    pthread_mutex_unlock(&mtx);

                } else {
                    int num_requests_to_drop = (int) ((waiting_request_queue->size + 1) / 2);
                    for (int i = 0; i < num_requests_to_drop; i++) {
                        if (waiting_request_queue->size != 0) {
                            int random_index = rand() % (waiting_request_queue->size);
                            QueueNODE *my_node = findNodeByIdx(waiting_request_queue, random_index);
                            int to_die=my_node->fd;
                            removeNodeFromQ(waiting_request_queue, my_node);
                            Close(to_die);
                        } else {
                               break;
                           }
                        }
                    ///deleted at least 1
                    insert(waiting_request_queue, connfd, arrival_time);
                    pthread_cond_signal(&cond);
                    pthread_mutex_unlock(&mtx);
                    }

                }
        }else {
                insert(waiting_request_queue, connfd, arrival_time);
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mtx);
        }
    }
}







