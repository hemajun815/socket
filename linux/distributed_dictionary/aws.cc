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

#define SERVERIP            "127.0.0.1" // server ip
#define UDPPORT_A           21146       // udp port for backend server A.
#define UDPPORT_B           22146       // udp port for backend server B.
#define UDPPORT_C           23146       // udp port for backend server C.
#define TCPPORTFORCLIENT    25146       // tcp port for client.
#define TCPPORTFORMONITOR   26146       // tcp port for monitor.

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
  Description: [Business] process request.
*************************************************/
void process_request(const int & port, const FUNCTION & func, const char * input, int * out_matches, char ** out_result)
{
    if (!out_result)
        return;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 != sockfd)
    {
        // send to...
        sockaddr_in addr_server;
        bzero(&addr_server, sizeof(sockaddr_in));
        addr_server.sin_family = AF_INET;
        addr_server.sin_addr.s_addr = inet_addr(SERVERIP);
        addr_server.sin_port = htons(port);
        socklen_t len = sizeof(sockaddr_in);
        char * buf_send = new char[28];
        sprintf(buf_send, "%d%s", func, input);
        sendto(sockfd, buf_send, 28, 0, (sockaddr *)&addr_server, len);

        // recv from...
        char ch_cmd;
        recvfrom(sockfd, &ch_cmd, 1, 0, (sockaddr *)&addr_server, &len);
        char str_matches_count[8];
        recvfrom(sockfd, str_matches_count, 8, 0, (sockaddr *)&addr_server, &len);
        *out_matches = str_to_int(str_matches_count);
        char str_len_result[8];
        recvfrom(sockfd, str_len_result, 8, 0, (sockaddr *)&addr_server, &len);
        int len_result = str_to_int(str_len_result);
        *out_result = new char[len_result + 1];
        bzero(*out_result, len_result + 1);
        int l = recvfrom(sockfd, *out_result, len_result, 0, (sockaddr *)&addr_server, &len);
        (*out_result)[l] = '\0';
    }
    return;
}

/*************************************************
  Description： [Business] entry.
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
            
            // process request
            int matches_count = 0;
            char * result = new char();
            process_request(UDPPORT_A, func, buf_recv, &matches_count, &result);
            printf("The AWS sent <%s> and <%s> to Backend-Server A", buf_recv, func == FUNC_SEARCH ? "search" : "prefix");
            delete buf_recv;

            // send response
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
