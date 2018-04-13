/*************************************************
  Filename: serverB.cc
  Creator: Hemajun
  Description: backend server B
*************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define UDPPORT             21146           // udp port.
#define SERVERNAME          "ServerA"       // server name.
#define FILENAME            "backendA.txt"  // data file.
#define MAXLINECOUNT        1024            // max line count.

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
  Description: [Helper] parse int to 8-len string.
*************************************************/
const char * int_to_str(const int & n)
{
    char * number = new char[8];
    bzero(number, 8);
    sprintf(number, "%8d", n);
    return number;
}

/*************************************************
  Description: [Helper] substring.
*************************************************/
const char * substring(const char * src, const int & start, const int & len = -1)
{
    int len_src = strlen(src);
    int len_sub = len > 0 ? len : len_src - start;
    if (len_sub > len_src)
        return "";
    char * sub_str = new char[len_sub + 1];
    for (int i = 0; i < len_sub; i++)
        sub_str[i] = src[i + start];
    sub_str[len_sub] = '\0';
    return sub_str;
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
  Description: [Business] search definition by key.
*************************************************/
void search(const char * input, int * out_matches_count, char ** out_definition, int * out_one_edit_count, 
            char ** out_one_edit, char ** out_one_edit_definition)
{
    *out_matches_count = 0;
    *out_one_edit_count = 0;
    int len_input = strlen(input);
    if (len_input == 0 || !out_definition)
        return;
    FILE * fp = fopen(FILENAME, "r");
    if (!fp)
        return;
    char * key = new char[len_input + 4];
    strcpy(key, input);
    strcpy(key + len_input, " :: ");
    char * buf = new char[MAXLINECOUNT];
    bzero(buf, MAXLINECOUNT);
    while(fgets(buf, MAXLINECOUNT, fp))
    {
        // match
        if (!*out_matches_count && !strcmp(key, substring(buf, 0, len_input + 4)))
        {
            int len_definition = strlen(buf) - (len_input + 4) + 2;
            *out_definition = new char[len_definition];
            (*out_definition)[0] = '<';
            strcpy((*out_definition) + 1, buf + len_input + 4);
            strcpy((*out_definition) + len_definition - 2, ">\n");
            *out_matches_count = 1;
        }
        // one-edit
        if (!*out_one_edit_count)
        {
            int edit = 0;
            for (int i = 0; i < len_input; i++)
            {
                if (input[i] != buf[i])
                    ++edit;
                if (edit > 1)
                    break;
            }
            if (edit == 1 && !strcmp(substring(buf, len_input, 4), " :: "))
            {
                *out_one_edit = new char[len_input + 2];
                (*out_one_edit)[0] = '<';
                strcpy((*out_one_edit) + 1, substring(buf, 0, len_input));
                (*out_one_edit)[len_input + 1] = '>';
                int len_one_edit_definition = strlen(buf) - (len_input + 4) + 2;
                *out_one_edit_definition = new char[len_one_edit_definition];
                (*out_one_edit_definition)[0] = '<';
                strcpy((*out_one_edit_definition) + 1, buf + len_input + 4);
                strcpy((*out_one_edit_definition) + len_one_edit_definition - 2, ">\n");
                *out_one_edit_count = 1;
            }
        }
        if (*out_matches_count && *out_one_edit_count)
            break;
        bzero(buf, MAXLINECOUNT);
    }
    fclose(fp);
    delete key;
    delete buf;
    return;
}

/*************************************************
  Description: [Business] search keys by prefix.
*************************************************/
void prefix(const char * input, int * out_count, char ** out_string)
{
    *out_count = 0;
    int len_input = strlen(input);
    if (len_input == 0 || !out_string)
        return;
    FILE * fp = fopen(FILENAME, "r");
    if (!fp)
        return;
    char * key = new char[len_input + 4];
    strcpy(key, input);
    strcpy(key + len_input, " :: ");
    char * buf = new char[MAXLINECOUNT];
    bzero(buf, MAXLINECOUNT);
    while(fgets(buf, MAXLINECOUNT, fp))
    {
        if (!strcmp(input, substring(buf, 0, len_input)) && strcmp(key, substring(buf, 0, len_input + 4)))
        {
            char * p = strstr(buf, " :: ");
            char * q = buf;
            int i = 0;
            while(q != p)
            {
                ++q;
                ++i;
            }
            int len_word = i + 3;
            char * word = new char[len_word];
            word[0] = '<';
            strcpy(word + 1, substring(buf, 0, i));
            strcpy(word + len_word - 2, ">\n");
            int len_string = strlen(*out_string);
            char * new_string = new char[len_string + len_word];
            strcpy(new_string, *out_string);
            strcpy(new_string + len_string, word);
            delete *out_string;
            *out_string = new_string;
            ++(*out_count);
        }
    }
    fclose(fp);
    delete key;
    delete buf;
    return;
}

/*************************************************
  Description: [Business] process request and send back response.
*************************************************/
void process_request(const int & sockfd)
{
    // request
    char ch_cmd;
    sockaddr_in addr_aws;
    socklen_t len_addr = sizeof(sockaddr_in);
    recvfrom(sockfd, &ch_cmd, 1, 0, (sockaddr *)&addr_aws, &len_addr);
    FUNCTION func = FUNCTION(str_to_int(&ch_cmd));
    int len_input = 27;
    char * input = new char[len_input + 1];
    bzero(input, len_input);
    int len = recvfrom(sockfd, input, len_input, 0, (sockaddr *)&addr_aws, &len_addr);
    input[len] = '\0';
    printf("The %s received input <%s> and operation <%s>.\n", SERVERNAME, input, func == FUNC_SEARCH ? "search" : "prefix");

    // response
    switch (func)
    {
        case FUNC_SEARCH:
        {
            int matches_count = 0;
            char * definition = new char();
            int one_edit_count = 0;
            char * one_edit = new char();
            char * one_edit_definition = new char();
            search(input, &matches_count, &definition, &one_edit_count, &one_edit, &one_edit_definition);
            sendto(sockfd, int_to_str(matches_count), 8, 0, (sockaddr*)&addr_aws, len_addr);
            if (matches_count)
            {
                int len_definition = strlen(definition);
                sendto(sockfd, int_to_str(len_definition), 8, 0, (sockaddr*)&addr_aws, len_addr);
                sendto(sockfd, definition, len_definition, 0, (sockaddr*)&addr_aws, len_addr);
            }
            sendto(sockfd, int_to_str(one_edit_count), 8, 0, (sockaddr*)&addr_aws, len_addr);
            if (one_edit_count)
            {
                int len_one_edit = strlen(one_edit);
                sendto(sockfd, int_to_str(len_one_edit), 8, 0, (sockaddr*)&addr_aws, len_addr);
                sendto(sockfd, one_edit, len_one_edit, 0, (sockaddr*)&addr_aws, len_addr);
                int len_one_edit_definition = strlen(one_edit_definition);
                sendto(sockfd, int_to_str(len_one_edit_definition), 8, 0, (sockaddr*)&addr_aws, len_addr);
                sendto(sockfd, one_edit_definition, len_one_edit_definition, 0, (sockaddr*)&addr_aws, len_addr);
            }
        }
        break;
        case FUNC_PREFIX:
        {
            int prefix_count = 0;
            char * prefix_string = new char();
            prefix(input, &prefix_count, &prefix_string);
            sendto(sockfd, int_to_str(prefix_count), 8, 0, (sockaddr*)&addr_aws, len_addr);
            if (prefix_count)
            {
                int len_prefix_string = strlen(prefix_string);
                sendto(sockfd, int_to_str(len_prefix_string), 8, 0, (sockaddr*)&addr_aws, len_addr);
                sendto(sockfd, prefix_string, len_prefix_string, 0, (sockaddr*)&addr_aws, len_addr);
            }
        }
        break;
    }
    printf("The %s finished sending the output to AWS.\n", SERVERNAME);
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
        fprintf(stderr, "Boot backend %s failure.\n", SERVERNAME);
        return -1;
    }
    printf("The %s is up and running using UDP on port <%d>.\n", SERVERNAME, UDPPORT);

    // whole loop
    while(1)
        process_request(sockfd);
    return 0;
}
