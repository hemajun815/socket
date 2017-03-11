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
    SOCKET sockServer = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN addrServer;
    memset(&addrServer, 0, sizeof(SOCKADDR_IN));
    addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(12345);
    if(-1 == bind(sockServer, (SOCKADDR *)&addrServer, sizeof(SOCKADDR)))
    {
        WSACleanup();
        return -1;
    }

    if(-1 == listen(sockServer, 10))
    {
        WSACleanup();
        return -1;
    }

    SOCKADDR_IN addrClient;
    int lenAddr = sizeof(SOCKADDR_IN);
    memset(&addrClient, 0, lenAddr);
    SOCKET sockClient = accept(sockServer, (SOCKADDR*)&addrClient, &lenAddr);
    if(-1 != sockClient)
    {
	    printf("Accepted a connection of %s.\n", inet_ntoa(addrClient.sin_addr));
    }
    closesocket(sockServer);
    WSACleanup();
}