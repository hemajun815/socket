#include <stdio.h>
#include <string.h>
#include <Winsock2.h>

int main(int argc, char const *argv[])
{
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;
    if(WSAStartup(wVersionRequested, &wsaData))
    {
        return -1;
    }
    if(1 != LOBYTE(wsaData.wVersion) || 1 != HIBYTE(wsaData.wVersion))
    {
        WSACleanup();
        return -1;
    }
    SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN addrServer;
    memset(&addrServer, 0, sizeof(SOCKADDR_IN));
    addrServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(12345);
    if(-1 != connect(sockClient, (SOCKADDR *)&addrServer, sizeof(SOCKADDR)))
    {
        printf("Connect to %s.\n", inet_ntoa(addrServer.sin_addr));
    }

    closesocket(sockClient);
    WSACleanup();
}