/*************************************************
  Filename: linked_list.cc
  Creator: Hemajun
  Description: Linked list to store online users.
*************************************************/
#include "linked_list.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t mtx_list = PTHREAD_MUTEX_INITIALIZER;

void append_user(PList & p_list, const PUser & p_user)
{
    pthread_mutex_lock(&mtx_list);
    PUserNode p_new_node = new UserNode();
    p_new_node->p_user = p_user;
    p_new_node->p_next = NULL;
    if (!p_list)
    {
        p_list = p_new_node;
    }
    else
    {
        PUserNode p_node = p_list;
        while (p_node->p_next)
        {
            p_node = p_node->p_next;
        }
        p_node->p_next = p_new_node;
    }
    pthread_mutex_unlock(&mtx_list);
}

void append_user(PList & p_list, const int & sockfd, const char * p_name)
{
    PUser p_user = new User();
    p_user->sockfd = sockfd;
    p_user->p_name = new char[strlen(p_name)];
    memcpy((void*)p_user->p_name, (void*)p_name, strlen(p_name));
    append_user(p_list, p_user);
}

PUser get_user(const PList & p_list, const int & sockfd)
{
    PUserNode p_node = p_list;
    while (p_node)
    {
        if (sockfd == p_node->p_user->sockfd)
        {
            return p_node->p_user;
        }
        p_node = p_node->p_next;
    }
    return NULL;
}

void destory(PList & p_list)
{
    PUserNode p_node = p_list;
    while (p_node)
    {
        PUserNode q_node = p_node->p_next;
        delete p_node->p_user->p_name;
        delete p_node->p_user;
        delete p_node;
        p_node = q_node;
    }
    p_list = NULL;
}