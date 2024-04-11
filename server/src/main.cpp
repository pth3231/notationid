#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "8080"
#define DEFAULT_BUFLEN 512

void _Log(const char *message)
{
    printf("------------- %s -------------\n", message);
}

int main()
{
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    _Log("WSAStartup() Success");

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create SOCKET for server to listen for client connections
    SOCKET listen_socket = INVALID_SOCKET;
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Print out IPV4 or IPV6 address
    // 192.168.1.1
    for (struct addrinfo *i = result; i != NULL; i = i->ai_next)
    {
        if (i->ai_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *p = (struct sockaddr_in *)i->ai_addr;
            printf("IPv4 address: %s\n", inet_ntoa(p->sin_addr));
        }
        else if (i->ai_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6 *p = (struct sockaddr_in6 *)i->ai_addr;
            char str[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &p->sin6_addr, str, sizeof(str));
            printf("IPv6 address: %s\n", str);
        }
    }

    // Setup TCP listening socket
    iResult = bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(listen_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    printf("socket is listening at port %s...\n", DEFAULT_PORT);

    // Create another SOCKET to handle client connection
    SOCKET accept_socket = INVALID_SOCKET;
    accept_socket = accept(listen_socket, NULL, NULL);
    if (accept_socket == INVALID_SOCKET)
    {
        printf("failed to accept: %s\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    char recvbuf[DEFAULT_BUFLEN] = {'3'};
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    do
    {
        iResult = recv(accept_socket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            printf("Bytes received: %d\n", iResult);

            // Echo the buffer back to the sender
            int iSendResult = send(accept_socket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(accept_socket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(accept_socket);
            WSACleanup();
            return 1;
        }
    } while (iResult > 0);

    // Shutdown the connection
    iResult = shutdown(accept_socket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(accept_socket);
        WSACleanup();
        return 1;
    }

    _Log("WSAStartup() Closed");
    closesocket(listen_socket);
    WSACleanup();
    return 0;
}