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

#define PORT 12345
#define MAX_DATA_SIZE 1024

#define ERROR(func_name) (printf("ERROR in function '%s'.\n", (func_name)))

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

int main(int argc, char const *argv[])
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
    addr_server.sin_port = htons(PORT);
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
    printf("Server started.\n");
    sockaddr_in addr_client;
    socklen_t socklen_client = sizeof(addr_client);
    int connfd = accept(sockfd, (sockaddr *)&addr_client, &socklen_client);
    if (-1 == connfd)
    {
        ERROR("accept");
        return -1;
    }
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
    close(sockfd);
    printf("Server stopped.\n");
    return 0;
}