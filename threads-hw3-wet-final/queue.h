//
// Created by student on 12/27/22.
//

#ifndef WEBSERVER_FILES_QUEUE_H
#define WEBSERVER_FILES_QUEUE_H
#include <stdbool.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>


///////////////////////////////////////// QUEUE NODE
typedef struct QueueNODE{
    struct timeval arrival;
    struct QueueNODE* prev;
    struct QueueNODE* next;
    int fd;
    struct timeval interval;

}QueueNODE;
///////////////////////////////////////// QUEUE CLASS
struct Queue{
    int size;
    QueueNODE* head;
    QueueNODE* tail;
    int max_size;
}typedef Queue;


Queue* create_queue(int max_size);
QueueNODE* node_create(struct timeval arrival,int fd);
QueueNODE* insert(Queue* queue, int fd, struct timeval arrival_time);
///we dont delete it
QueueNODE* get_tail_and_remove(Queue* queue);
void removeNodeFromQ(Queue *QUEUE,QueueNODE * ode);
QueueNODE* findNodeInQ(Queue *QUEUE,int key);


QueueNODE* findNodeByIdx(Queue *QUEUE,int index);


#endif //WEBSERVER_FILES_QUEUE_H