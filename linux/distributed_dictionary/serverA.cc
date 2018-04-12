/*************************************************
  Filename: serverA.cc
  Creator: Hemajun
  Description: backend server A.
*************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define UDPPORT             21146 // udp port.

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
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 != sockfd)
    {
        sockaddr_in addr_server;
        bzero(&addr_server, sizeof(sockaddr_in));
        addr_server.sin_family = AF_INET;
        addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
        addr_server.sin_port = htons(port);
        if (-1 != bind(sockfd, (sockaddr *)&addr_server, sizeof(sockaddr_in)))
            return sockfd;
        close(sockfd);
    }
    return -1;
}

/*************************************************
  Description: [Business] process request and send back response.
*************************************************/
void process_request(const int & sockfd)
{
    // recv request
    char ch_cmd;
    sockaddr_in addr_aws;
    socklen_t len_addr = sizeof(sockaddr_in);
    recvfrom(sockfd, &ch_cmd, 1, 0, (sockaddr *)&addr_aws, &len_addr);
    FUNCTION func = FUNCTION(str_to_int(&ch_cmd));
    int len_input = 27;
    char * buf_recv = new char[len_input + 1];
    bzero(buf_recv, len_input);
    int len = recvfrom(sockfd, buf_recv, len_input, 0, (sockaddr *)&addr_aws, &len_addr);
    buf_recv[len] = '\0';
    printf("The ServerA received input <%s> and operation <%s>.\n", buf_recv, func == FUNC_SEARCH ? "search" : "prefix");
    delete buf_recv;

    // send response
    const int matches_count = 1;
    const char * result = "<test>\n";
    int len_result = strlen(result);
    char * buf_send = new char[1 + 8 + 8 + len_result];
    bzero(buf_send, 1 + 8 + 8 + len_result);
    sprintf(buf_send, "%d%8d%8d%s", func, matches_count, len_result, result);
    sendto(sockfd, buf_send, 1 + 8 + 8 + len_result, 0, (sockaddr *)&addr_aws, len_addr);
    delete buf_send;
    printf("The ServerA finished sending the output to AWS.\n");
}

/*************************************************
  Description： [Business] entry.
*************************************************/
int main(int argc, char const *argv[])
{
    // start server
    int sockfd = start_server(UDPPORT);
    if (-1 == sockfd)
    {
        fprintf(stderr, "Boot backend server A failure.\n");
        return -1;
    }
    printf("The ServerA is up and running using UDP on port <%d>.\n", UDPPORT);

    // whole loop
    while(1)
        process_request(sockfd);

    return 0;
}
