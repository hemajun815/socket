/*************************************************
  Filename: monitor.cc
  Creator: Hemajun
  Description: monitor
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
  Description： [Business] entry.
*************************************************/
int main(int argc, char const *argv[])
{
    // connect to aws.
    int sockfd = connect_to_server(SERVERIP, SERVERPORT);
    if (-1 == sockfd)
    {
        fprintf(stderr, "Connect AWS failure.\n");
        return -1;
    }
    printf("The monitor is up and running.\n");

    // whole loop
    while (1)
    {
        char ch_cmd;
        recv(sockfd, &ch_cmd, 1, 0);
        switch ((FUNCTION)str_to_int(&ch_cmd))
        {
            case FUNC_SEARCH:
            {
                char number[8];
                bzero(number, 8);
                recv(sockfd, number, 8, 0);
                int len_definition = str_to_int(number);
                if (len_definition)
                {
                    char * buf = new char[len_definition];
                    recv(sockfd, buf, len_definition, 0);
                    printf("Found a match:\n%s", buf);
                    delete buf;
                }
                else
                    printf("Found no match.\n");
                bzero(number, 8);
                recv(sockfd, number, 8, 0);
                int len_one_edit = str_to_int(number);
                if (len_one_edit)
                {
                    char * buf_one_edit = new char[len_one_edit];
                    recv(sockfd, buf_one_edit, len_one_edit, 0);
                    bzero(number, 8);
                    recv(sockfd, number, 8, 0);
                    int len_one_edit_definition = str_to_int(number);
                    char * buf_one_edit_definition = new char[len_one_edit_definition];
                    recv(sockfd, buf_one_edit_definition, len_one_edit_definition, 0);
                    printf("One edit distance match is %s:\n%s", buf_one_edit, buf_one_edit_definition);
                    delete buf_one_edit;
                    delete buf_one_edit_definition;
                }
                else
                    printf("No one edit distance match.\n");
            }
            break;
            case FUNC_PREFIX:
            {
                char number[8];
                bzero(number, 8);
                recv(sockfd, number, 8, 0);
                int prefix_count = str_to_int(number);
                if (prefix_count)
                {
                    bzero(number, 8);
                    recv(sockfd, number, 8, 0);
                    int len_prefix_string = str_to_int(number);
                    char * buf = new char[len_prefix_string];
                    recv(sockfd, buf, len_prefix_string, 0);
                    printf("Found <%d> match(es):\n%s", prefix_count, buf);
                    delete buf;
                }
                else
                    printf("Found no match for <%s>:\n", argv[2]);
            }
            break;
        }
    }
    return 0;
}
