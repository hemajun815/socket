/*************************************************
  Filename: client.cc
  Creator: Hemajun
  Description: client
*************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVERIP    "127.0.0.1" // server ip
#define SERVERPORT  25146       // server port

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
  Description： [Business] show usage.
*************************************************/
void show_usage(const char * arg0)
{
    printf("Usage: %s <function> <input>\n", arg0);
    printf(" - function: string, must be one of 'search' and 'prefix'.\n");
    printf(" - input: string, must be a word consisting of 27 characters or less.\n");
}

/*************************************************
  Description： [Business] connect to server. 
  if success, return sockfd; else return -1.
*************************************************/
int connect_to_server(const char * server_ip, const int & server_port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 != sockfd)
    {
        sockaddr_in addr_server;
        bzero(&addr_server, sizeof(sockaddr_in));
        addr_server.sin_family = AF_INET;
        inet_aton(server_ip, &addr_server.sin_addr);
        addr_server.sin_port = htons(server_port);
        if (-1 != connect(sockfd, (sockaddr *)&addr_server, sizeof(sockaddr_in)))
        {
            return sockfd;
        }
    }
    return -1;
}

/*************************************************
  Description: [Business] preocess response.
*************************************************/
void process_response(const int & sockfd, FUNCTION func, const char * input)
{
    char str_matches_count[8], str_len_result[8];
    recv(sockfd, str_matches_count, 8, 0);
    int matches_count = str_to_int(str_matches_count);
    if (matches_count == 0)
    {
        printf("Found no matches for '%s'.\n", input);
        return;
    }
    else if (matches_count == 1)
        printf("Found a match for <%s>:\n", input);
    else
        printf("Found <%d> matches for <%s>:\n", matches_count, input);
    recv(sockfd, str_len_result, 8, 0);
    int len_result = str_to_int(str_len_result);
    char * buf = new char[len_result + 1];
    bzero(buf, len_result + 1);
    int len = recv(sockfd, buf, len_result, 0);
    buf[len] = '\0';
    printf("%s", buf);
    delete buf;
}

/*************************************************
  Description： [Business] entry.
*************************************************/
int main(int argc, char const *argv[])
{
    // parameters
    if (argc != 3 || (strcmp("search", argv[1]) && strcmp("prefix", argv[1])) || strlen(argv[2]) > 27)
    {
        show_usage(argv[0]);
        return -1;
    }
    int cmd = !strcmp("search", argv[1]) ? FUNC_SEARCH : FUNC_PREFIX;
    char request[28];
    sprintf(request, "%d%s", cmd, argv[2]);

    // connect to aws.
    int sockfd = connect_to_server(SERVERIP, SERVERPORT);
    if (-1 == sockfd)
    {
        fprintf(stderr, "Connect AWS failure.\n");
        return -1;
    }
    printf("The client is up and running.\n");

    // send request
    send(sockfd, request, 28, 0);
    printf("The client sent <%s> and <%s> to AWS.\n", argv[2], argv[1]);

    // recv response
    char ch_cmd;
    recv(sockfd, &ch_cmd, 1, 0);
    process_response(sockfd, (FUNCTION)str_to_int(&ch_cmd), argv[2]);

    // close connection.
    close(sockfd);
    return 0;
}
