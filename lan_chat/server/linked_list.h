/*************************************************
  Filename: link_list.h
  Creator: Hemajun
  Description: Linked list to store online users.
*************************************************/
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct User
{
    int sockfd;
    char * p_name;
}User, *PUser;

typedef struct UserNode
{
    PUser p_user;
    struct UserNode * p_next;
}UserNode, *PUserNode, *PList;

void append_user(PList & p_list, const int & sockfd, const char * p_name);
PUser get_user(const PList & p_list, const int & sockfd);
void destory(PList & p_list);

#endif