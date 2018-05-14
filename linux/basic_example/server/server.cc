/*************************************************
  Filename: server.cc
  Creator: Hemajun
  Description: linux socket server
*************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 12345
#define MAX_DATA_SIZE 1024

#define ERROR(func_name) (printf("ERROR in function '%s'.\n", (func_name)))

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct connection_info
{
    int connfd;
    sockaddr_in addr;
}connection_info;

int start_server(const int& port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        ERROR("socket");
        return -1;
    }
    sockaddr_in addr_server;
    bzero(&addr_server, sizeof(sockaddr_in));
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_server.sin_port = htons(port);
    if (-1 == bind(sockfd, (sockaddr *)&addr_server, sizeof(sockaddr_in)))
    {
        ERROR("bind");
        return -1;
    }
    if (-1 == listen(sockfd, 8))
    {
        ERROR("listen");
        return -1;
    }
    return sockfd;
}

void communicate(const int& connfd, const sockaddr_in& addr_client)
{
    printf("Accepted a connection of %s.\n", inet_ntoa(addr_client.sin_addr));
    char *p_buffer = new char[MAX_DATA_SIZE];
    while (true)
    {
        int len_data = recv(connfd, p_buffer, MAX_DATA_SIZE, 0);
        if (0 < len_data)
        {
            p_buffer[len_data] = '\0';
            printf("Received data: %s.\n", p_buffer);
            if (-1 != send(connfd, p_buffer, strlen(p_buffer), 0))
                printf("Sent data: %s.\n", p_buffer);
        }
        else
            break;
    }
    delete p_buffer;
    close(connfd);
}

void * thread_routine(void * thread_arg)
{
    connection_info * p_c_info = (connection_info *)thread_arg;
    communicate(p_c_info->connfd, p_c_info->addr);
    delete p_c_info;
    pthread_exit(NULL);
}

void wait_connect(const int & sockfd)
{
    while (true)
    {
        sockaddr_in addr_client;
        socklen_t socklen_client = sizeof(addr_client);
        int connfd = accept(sockfd, (sockaddr *)&addr_client, &socklen_client);
        if (-1 == connfd)
            ERROR("accept");
        else
        {
            pthread_t tid = 0;
            connection_info * p_c_info = new connection_info();
            p_c_info->connfd = connfd;
            memcpy((void*)&(p_c_info->addr), (void*)&addr_client, socklen_client);
            if (-1 == pthread_create(&tid, NULL, thread_routine, (void *)p_c_info))
                ERROR("pthread_create");
        }
    }
}

int main(int argc, char const *argv[])
{
    int sockfd = start_server(PORT);
    if (-1 == sockfd)
        return -1;
    printf("Server started.\n");
    wait_connect(sockfd);
    close(sockfd);
    printf("Server stopped.\n");
    return 0;
}