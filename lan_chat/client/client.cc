/*************************************************
  Filename: client.cc
  Creator: Hemajun
  Description: Client of LAN_Chat.
*************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../public/config.h"
#include "../public/protocol.h"

int connect_server(const char * server_ip, const int & port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 != sockfd)
    {
        sockaddr_in addr_server;
        bzero(&addr_server, sizeof(sockaddr_in));
        addr_server.sin_family = AF_INET;
        inet_aton(server_ip, &addr_server.sin_addr);
        addr_server.sin_port = htons(port);
        if (-1 != connect(sockfd, (sockaddr *)&addr_server, sizeof(sockaddr_in)))
        {
            return sockfd;
        }
    }
    return -1;
}

void communicate(const int& sockfd)
{
    printf("Connected server succesfully.\n");
    char *p_buffer = new char[TCP_MAX_DATA_SIZE];
    while (true)
    {
        int idx = 0;
        char ch;
        printf("Please input your data: ");
        while((ch = getchar()) != '\n')
            p_buffer[idx++] = ch;
        p_buffer[idx] = '\0';
        if (-1 != send(sockfd, p_buffer, strlen(p_buffer), 0))
        {
            printf("Sent data: %s\n", p_buffer);
            bzero(p_buffer, TCP_MAX_DATA_SIZE);
            int len_data = recv(sockfd, p_buffer, TCP_MAX_DATA_SIZE, 0);
            if (0 < len_data)
            {
                p_buffer[len_data] = '\0';
                printf("Received data: %s\n", p_buffer);
            }
        }
    }
    delete p_buffer;
}

void login(const int & sockfd)
{
    PPackage p_package = new Package();
    p_package->package_type = PackageTypeRequestLogin;
    char *p_buffer = new char[TCP_MAX_DATA_SIZE];
    bzero(p_buffer, TCP_MAX_DATA_SIZE);
    int idx = 0;
    char ch;
    printf("Please input your name: ");
    while((ch = getchar()) != '\n')
        p_buffer[idx++] = ch;
    p_buffer[idx] = '\0';
    p_package->sender_name = new char[idx];
    memcpy(p_package->sender_name, p_buffer, idx);
    const char * null_str = "";
    int len_null_str = strlen(null_str);
    p_package->receiver_name = new char[len_null_str + 1];
    memcpy(p_package->receiver_name, null_str, len_null_str);
    p_package->receiver_name[len_null_str] = '\0';
    p_package->data = new char[len_null_str];
    memcpy(p_package->data, null_str, len_null_str);
    p_package->data[len_null_str] = '\0';
    char * p_json = encode_package(p_package);
    bzero(p_buffer, TCP_MAX_DATA_SIZE);
    memcpy(p_buffer, p_json, strlen(p_json));
    p_buffer[strlen(p_json)] = '\0';
    if (-1 != send(sockfd, p_buffer, strlen(p_buffer), 0))
    {
        printf("Sent: %s\n", p_buffer);
        bzero(p_buffer, TCP_MAX_DATA_SIZE);
        int len_data = recv(sockfd, p_buffer, TCP_MAX_DATA_SIZE, 0);
        if (0 < len_data)
        {
            p_buffer[len_data] = '\0';
            printf("Received: %s\n", p_buffer);
        }
    }
}

int main(int argc, char const *argv[])
{
    if (2 != argc)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }
    int sockfd = connect_server(argv[1], TCP_SERVER_PORT);
    if (-1 != sockfd)
    {
        printf("Connected server succesfully.\n");
        // communicate(sockfd);
        login(sockfd);
        close(sockfd);
        return 0;
    }
    return -1;
}