/*************************************************
  Filename: server.cc
  Creator: Hemajun
  Description: Server of LAN_Chat.
*************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "linked_list.h"
#include "fifo_queue.h"
#include "../public/config.h"
#include "../public/protocol.h"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

PList users = NULL;
PQueue requests = NULL;
PQueue responces = NULL;

int start_server(const int& port, const int& backlog)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 != sockfd)
    {
        sockaddr_in addr_server;
        bzero(&addr_server, sizeof(sockaddr_in));
        addr_server.sin_family = AF_INET;
        addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
        addr_server.sin_port = htons(port);
        if (-1 != bind(sockfd, (sockaddr *)&addr_server, sizeof(sockaddr_in)) && -1 != listen(sockfd, backlog))
        {
            return sockfd;
        }
        close(sockfd);
    }
    return -1;
}

void send_responce(void* (*thread_routine)(void*))
{
    pthread_t tid = 0;
    pthread_create(&tid, NULL, thread_routine, NULL);
}

void process_request(void* (*thread_routine)(void*))
{
    pthread_t tid = 0;
    pthread_create(&tid, NULL, thread_routine, NULL);
}

void wait_connect(const int & sockfd, void* (*thread_routine)(void*))
{
    while (true)
    {
        sockaddr_in addr_client;
        socklen_t socklen_client = sizeof(addr_client);
        int connfd = accept(sockfd, (sockaddr *)&addr_client, &socklen_client);
        if (-1 != connfd)
        {
            pthread_t tid = 0;
            pthread_create(&tid, NULL, thread_routine, (void *)&connfd);
        }
    }
}

void * send_thread_routine(void * thread_arg)
{
    while (true)
    {
        PDataPackage p_data_package = pop(responces);
        send(p_data_package->sockfd, p_data_package->p_data, strlen(p_data_package->p_data), 0);
        printf("Sent: %s\n", p_data_package->p_data);
    }
    pthread_exit(NULL);
}

void * process_thread_rountine(void * thread_arg)
{
    while (true)
    {
        PDataPackage p_data_package = pop(requests);
        char * p_json = p_data_package->p_data;
        PPackage p_req_package = decode_package(p_json);
        PPackage p_res_package = new Package();
        switch (p_req_package->package_type)
        {
            case PackageTypeRequestLogin:
                p_res_package->package_type = PackageTypeResponceLogin;
                p_res_package->sender_name = new char[strlen(p_req_package->receiver_name) + 1];
                memcpy(p_res_package->sender_name, p_req_package->receiver_name, strlen(p_req_package->receiver_name));
                p_res_package->sender_name[strlen(p_req_package->receiver_name)] = '\0';
                p_res_package->receiver_name = new char[strlen(p_req_package->sender_name) + 1];
                memcpy(p_res_package->receiver_name, p_req_package->sender_name, strlen(p_req_package->sender_name));
                p_res_package->receiver_name[strlen(p_req_package->sender_name)] = '\0';
                const char * ret_str = "OK";
                int len_ret_str = strlen(ret_str);
                p_res_package->data = new char[strlen(ret_str) + 1];
                memcpy(p_res_package->data, ret_str, strlen(ret_str));
                p_res_package->data[len_ret_str] = '\0';
                break;
        }
        push(responces, p_data_package->sockfd, encode_package(p_res_package));
    }
    pthread_exit(NULL);
}

void * recv_thread_routine(void * thread_arg)
{
    int * p_connfd = (int *)thread_arg;
    char *p_buffer = new char[TCP_MAX_DATA_SIZE];
    while (true)
    {
        bzero(p_buffer, TCP_MAX_DATA_SIZE);
        int len_data = recv(*p_connfd, p_buffer, TCP_MAX_DATA_SIZE, 0);
        if (0 < len_data)
        {
            p_buffer[len_data] = '\0';
            printf("Received: %s\n", p_buffer);
            push(requests, *p_connfd, p_buffer);
        }
    }
    close(*p_connfd);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    int sockfd = start_server(TCP_SERVER_PORT, TCP_SERVER_BACKLOG);
    if (-1 != sockfd)
    {
        send_responce(send_thread_routine);
        process_request(process_thread_rountine);
        wait_connect(sockfd, recv_thread_routine);
        return 0;
    }
    return -1;
}