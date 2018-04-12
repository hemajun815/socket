/*************************************************
  Filename: aws.cc
  Creator: Hemajun
  Description: aws
*************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define UDPPORT             24146 // udp port.
#define TCPPORTFORCLIENT    25146 // tcp port for client.
#define TCPPORTFORMONITOR   26146 // tcp port for monitor.

enum FUNCTION {
    FUNC_SEARCH,
    FUNC_PREFIX
};

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

/*************************************************
  Description： [Helper] parse string to int.
*************************************************/
int str_to_int(const char * str)
{
    int num = 0;
    int len = strlen(str);
    for (int i = 0; i < len; i++) 
        if (str[i] >= '0' and str[i] <= '9')
            num = num * 10 + (str[i] - '0');
    return num;
}

/*************************************************
  Description： [Business] start server.
  if success, return sockfd; else return -1.
*************************************************/
int start_server(const int & port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 != sockfd)
    {
        sockaddr_in addr_server;
        bzero(&addr_server, sizeof(sockaddr_in));
        addr_server.sin_family = AF_INET;
        addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
        addr_server.sin_port = htons(port);
        if (-1 != bind(sockfd, (sockaddr *)&addr_server, sizeof(sockaddr_in)) && -1 != listen(sockfd, 1))
        {
            return sockfd;
        }
        close(sockfd);
    }
    return -1;
}

/*************************************************
  Description： [Helper] entry.
*************************************************/
int main(int argc, char const *argv[])
{
    // start server.
    int sock_client = start_server(TCPPORTFORCLIENT);
    int sock_monitor = start_server(TCPPORTFORMONITOR);
    if (-1 == sock_client || -1 == sock_monitor)
    {
        fprintf(stderr, "Boot AWS failure.\n");
        return -1;
    }
    printf("The AWS is up and running.\n");

    // whole loop.
    while (1)
    {
        int conn_client = -1; // , conn_monitor = -1;
        sockaddr_in addr_client; // , addr_monitor;
        socklen_t socklen_client = sizeof(addr_client); // , socklen_monitor = sizeof(addr_monitor);
        if (-1 != (conn_client = accept(sock_client, (sockaddr *)&addr_client, &socklen_client))
            )
            // && -1 != (conn_monitor = accept(sock_monitor, (sockaddr *)&addr_monitor, &socklen_monitor)))
        {
            // recv request
            char ch_cmd;
            recv(conn_client, &ch_cmd, 1, 0);
            FUNCTION func = FUNCTION(str_to_int(&ch_cmd));
            int len_input = 27;
            char * buf_recv = new char[len_input + 1];
            bzero(buf_recv, len_input);
            int len = recv(conn_client, buf_recv, len_input, 0);
            buf_recv[len] = '\0';
            printf("The AWS received input=<%s> and function=<%s> from the client using TCP over port %d.\n",
                    buf_recv, func == FUNC_SEARCH ? "search" : "prefix", TCPPORTFORCLIENT);
            delete buf_recv;

            // send response
            const int matches_count = 1;
            const char * result = "<test>\n";
            int len_result = strlen(result);
            char * buf_send = new char[1 + 8 + 8 + len_result];
            bzero(buf_send, 1 + 8 + 8 + len_result);
            sprintf(buf_send, "%d%8d%8d%s", func, matches_count, len_result, result);
            send(conn_client, buf_send, 1 + 8 + 8 + len_result, 0);
            delete buf_send;
            printf("The AWS sent <%d> matches to client.\n", matches_count);
        }
    }
    return 0;
}
