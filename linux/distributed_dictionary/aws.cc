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
// #define TCPPORTFORMONITOR   26146       // tcp port for monitor.

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
  Description: [Business] send udp request to backend server.
*************************************************/
void send_udp_request(const int & sockfd, const sockaddr_in & addr_server, const socklen_t & len, 
                      const FUNCTION & func, const char * input)
{
    char str_func[1];
    sprintf(str_func, "%d", func);
    sendto(sockfd, str_func, 1, 0, (sockaddr *)&addr_server, len);
    sendto(sockfd, input, 27, 0, (sockaddr *)&addr_server, len);
}

/*************************************************
  Description: [Business] search.
*************************************************/
void search(const int & port, const char * input, int * out_matches, char ** out_definition, int * out_one_edit_count, 
            char ** out_one_edit, char ** out_one_edit_definition)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr_server;
    bzero(&addr_server, sizeof(sockaddr_in));
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = inet_addr(SERVERIP);
    addr_server.sin_port = htons(port);
    socklen_t len = sizeof(sockaddr_in);

    send_udp_request(sockfd, addr_server, len, FUNC_SEARCH, input);

    char number[8];
    bzero(number, 8);
    recvfrom(sockfd, number, 8, 0, (sockaddr *)&addr_server, &len);
    *out_matches = str_to_int(number);
    if (*out_matches)
    {
        bzero(number, 8);
        recvfrom(sockfd, number, 8, 0, (sockaddr *)&addr_server, &len);
        int len_definition = str_to_int(number);
        *out_definition = new char[len_definition];
        recvfrom(sockfd, *out_definition, len_definition, 0, (sockaddr *)&addr_server, &len);
    }
    bzero(number, 8);
    recvfrom(sockfd, number, 8, 0, (sockaddr *)&addr_server, &len);
    *out_one_edit_count = str_to_int(number);
    if (*out_one_edit_count)
    {
        bzero(number, 8);
        recvfrom(sockfd, number, 8, 0, (sockaddr *)&addr_server, &len);
        int len_one_edit = str_to_int(number);
        *out_one_edit = new char[len_one_edit];
        recvfrom(sockfd, *out_one_edit, len_one_edit, 0, (sockaddr *)&addr_server, &len);
        bzero(number, 8);
        recvfrom(sockfd, number, 8, 0, (sockaddr *)&addr_server, &len);
        int len_one_edit_definition = str_to_int(number);
        *out_one_edit_definition = new char[len_one_edit_definition];
        recvfrom(sockfd, *out_one_edit_definition, len_one_edit_definition, 0, (sockaddr *)&addr_server, &len);
    }
}

/*************************************************
  Description: [Business] process search funtion.
*************************************************/
void process_search(const char * input, int * out_matches, char ** out_definition, int * out_one_edit_count, 
            char ** out_one_edit, char ** out_one_edit_definition)
{
    int tmp_matches = 0;
    char * tmp_definition = new char();
    int tmp_one_edit_count = 0;
    char * tmp_one_edit = new char();
    char * tmp_one_edit_definition = new char();
    int ports[] = {UDPPORT_A, UDPPORT_B, UDPPORT_C};
    for (int i = 0; i < 3; i++)
    {
        search(ports[i], input, &tmp_matches, &tmp_definition, &tmp_one_edit_count, &tmp_one_edit, 
               &tmp_one_edit_definition);
        if (tmp_matches)
        {
            *out_matches = tmp_matches;
            *out_definition = new char[strlen(tmp_definition)];
            strcpy(*out_definition, tmp_definition);
        }
        if (tmp_one_edit_count)
        {
            *out_one_edit_count = tmp_one_edit_count;
            *out_one_edit = new char[strlen(tmp_one_edit)];
            strcpy(*out_one_edit, tmp_one_edit);
            *out_one_edit_definition = new char[strlen(tmp_one_edit_definition)];
            strcpy(*out_one_edit_definition, tmp_one_edit_definition);
        }
        if (*out_matches && *out_one_edit_count)
            break;
    }
}

/*************************************************
  Description: [Business] prefix.
*************************************************/
void prefix(const int & port, const char * input, int * out_count, char ** out_string)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr_server;
    bzero(&addr_server, sizeof(sockaddr_in));
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = inet_addr(SERVERIP);
    addr_server.sin_port = htons(port);
    socklen_t len = sizeof(sockaddr_in);

    send_udp_request(sockfd, addr_server, len, FUNC_PREFIX, input);

    char number[8];
    bzero(number, 8);
    recvfrom(sockfd, number, 8, 0, (sockaddr *)&addr_server, &len);
    *out_count = str_to_int(number);
    if (*out_count)
    {
        bzero(number, 8);
        recvfrom(sockfd, number, 8, 0, (sockaddr *)&addr_server, &len);
        int len_prefix_string = str_to_int(number);
        *out_string = new char[len_prefix_string];
        recvfrom(sockfd, *out_string, len_prefix_string, 0, (sockaddr *)&addr_server, &len);
    }
}

/*************************************************
  Description: [Business] process prefix function.
*************************************************/
void process_prefix(const char * input, int * out_count, char ** out_string)
{
    int ports[] = {UDPPORT_A, UDPPORT_B, UDPPORT_C};
    char ** tmp_string = new char*[3];
    int len_string = 0;
    for (int i = 0; i < 3; i++)
    {
        int tmp_count = 0;
        prefix(ports[i], input, &tmp_count, &(tmp_string[i]));
        *out_count += tmp_count;
        len_string += strlen(tmp_string[i]);
    }
    *out_string = new char[len_string];
    int idx = 0;
    for (int i = 0; i < 3; i++)
    {
        strcpy((*out_string) + idx, tmp_string[i]);
        idx += strlen(tmp_string[i]);
        delete tmp_string[i];
    }
    delete[] tmp_string;
}

/*************************************************
  Description: [Business] start tcp server.
  if success, return sockfd; else return -1.
*************************************************/
int start_tcp_server(const int & port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 != sockfd)
    {
        const int yes = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
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
  Description： [Business] entry.
*************************************************/
int main(int argc, char const *argv[])
{
    // start server for client
    int sock_client = start_tcp_server(TCPPORTFORCLIENT);
    if (-1 == sock_client)
    {
        fprintf(stderr, "Boot TCP server failure.\n");
        return -1;
    }
    printf("The AWS is up and running.\n");

    // whole loop
    while (1)
    {
        int conn_client = -1;
        sockaddr_in addr_client;
        socklen_t socklen_client = sizeof(addr_client);
        if (-1 != (conn_client = accept(sock_client, (sockaddr *)&addr_client, &socklen_client)))
        {
            char ch_cmd;
            recv(conn_client, &ch_cmd, 1, 0);
            FUNCTION func = FUNCTION(str_to_int(&ch_cmd));
            const int len_input = 27;
            char * input = new char[len_input];
            bzero(input, len_input);
            recv(conn_client, input, len_input, 0);
            printf("The AWS received input=<%s> and function=<%s> from the client using TCP over port <%d>.\n",
                   input, func == FUNC_SEARCH ? "search" : "prefix", TCPPORTFORCLIENT);
            switch (func)
            {
                case FUNC_SEARCH:
                {
                    int matches_count = 0;
                    char * definition = new char();
                    int one_edit_count = 0;
                    char * one_edit = new char();
                    char * one_edit_definition = new char();
                    process_search(input, &matches_count, &definition, &one_edit_count, &one_edit, &one_edit_definition);
                    int len_res_client = -1;
                    char * res_client = NULL;
                    if (matches_count)
                    {
                        len_res_client = 1 + 8 + strlen(definition);
                        res_client = new char[len_res_client];
                        sprintf(res_client, "%d%8d%s", FUNC_SEARCH, (int)strlen(definition), definition);
                    }
                    else
                    {
                        len_res_client = 1 + 8;
                        res_client = new char[len_res_client];
                        sprintf(res_client, "%d%8d", FUNC_SEARCH, (int)strlen(definition));
                    }
                    send(conn_client, res_client, len_res_client, 0);
                    delete res_client;
                    printf("The AWS sent <%d> matches to client.\n", matches_count);
                    if (one_edit_count)
                        printf("One edit distance match is %s:\n%s", one_edit, one_edit_definition);
                }
                break;
                case FUNC_PREFIX:
                {
                    int prefix_count = 0;
                    char * prefix_string = new char();
                    process_prefix(input, &prefix_count, &prefix_string);
                    int len_res_client = -1;
                    char * res_client = NULL;
                    if (prefix_count)
                    {
                        len_res_client = 1 + 8 + 8 + strlen(prefix_string);
                        res_client = new char[len_res_client];
                        sprintf(res_client, "%d%8d%8d%s", FUNC_PREFIX, prefix_count, (int)strlen(prefix_string), 
                                prefix_string);
                    }
                    else
                    {
                        len_res_client = 1 + 8;
                        res_client = new char[len_res_client];
                        sprintf(res_client, "%d%8d", FUNC_PREFIX, prefix_count);
                    }
                    send(conn_client, res_client, len_res_client, 0);
                    delete res_client;
                    printf("The AWS sent <%d> matches to client.\n", prefix_count);
                }
                break;
            }
        }
    }
    close(sock_client);
    return 0;
}
