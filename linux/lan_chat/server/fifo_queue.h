/*************************************************
  Filename: fifo_queue.h
  Creator: Hemajun
  Description: FIFO blocking queue to store data package.
*************************************************/
#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

typedef struct DataPackage
{
    int sockfd;
    char * p_data;
}DataPackage, *PDataPackage;

typedef struct QueueNode
{
    PDataPackage p_data_package;
    struct QueueNode * p_next;
}QueueNode, *PQueueNode, *PQueue;

void push(PQueue & p_queue, const int & sockfd, const char * p_data);
PDataPackage pop(PQueue & p_queue);

#endif