/*************************************************
  Filename: client.cc
  Creator: Hemajun
  Description: linux socket client
*************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 12345

#define ERROR(func_name) (printf("ERROR in function '%s'.\n", (func_name)))

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

int main(int argc, char const *argv[])
{
    if (2 != argc)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        ERROR("socket");
        return -1;
    }
    sockaddr_in addr_server;
    bzero(&addr_server, sizeof(sockaddr_in));
    addr_server.sin_family = AF_INET;
    inet_aton(argv[1], &addr_server.sin_addr);
    addr_server.sin_port = htons(PORT);
    if (-1 == connect(sockfd, (sockaddr *)&addr_server, sizeof(sockaddr_in)))
    {
        ERROR("connect");
        return -1;
    }
    close(sockfd);
    return 0;
}