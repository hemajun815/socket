/*************************************************
  Filename: fifo_queue.cc
  Creator: Hemajun
  Description: FIFO blocking queue to store data package.
*************************************************/
#include "fifo_queue.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t mtx_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_queue = PTHREAD_COND_INITIALIZER;

void push(PQueue & p_queue, const PDataPackage & p_data_package)
{
    pthread_mutex_lock(&mtx_queue);
    PQueueNode p_new_node = new QueueNode();
    p_new_node->p_data_package = p_data_package;
    p_new_node->p_next = NULL;
    if (!p_queue)
    {
        p_queue = p_new_node;
    }
    else
    {
        PQueueNode p_queue_node = p_queue;
        while (p_queue_node->p_next)
        {
            p_queue_node = p_queue_node->p_next;
        }
        p_queue_node->p_next = p_new_node;
    }
    pthread_mutex_unlock(&mtx_queue);
    pthread_cond_broadcast(&cond_queue);
}

void push(PQueue & p_queue, const int & sockfd, const char * p_data)
{
    PDataPackage p_data_package = new DataPackage();
    p_data_package->sockfd = sockfd;
    p_data_package->p_data = new char[strlen(p_data)];
    memcpy((void*)p_data_package->p_data, (void*)p_data, strlen(p_data));
    push(p_queue, p_data_package);
}

PDataPackage pop(PQueue & p_queue)
{
    pthread_mutex_lock(&mtx_queue);
    while (!p_queue)
    {
        pthread_cond_wait(&cond_queue, &mtx_queue);
    }
    PQueueNode p_queue_node = p_queue;
    p_queue = p_queue->p_next;
    pthread_mutex_unlock(&mtx_queue);
    return p_queue_node->p_data_package;
}