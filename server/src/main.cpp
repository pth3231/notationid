#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include "../lib/cjson/cJSON.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "8080"
#define DEFAULT_BUFLEN 1024
#define IP_ADDR "127.0.0.1"

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

    if (getaddrinfo(IP_ADDR, DEFAULT_PORT, &hints, &result) != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create SOCKET for server to listen for client connections
    SOCKET listen_socket = INVALID_SOCKET;
    if ((listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Print out IPV4 or IPV6 address
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
    if (bind(listen_socket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
    {
        printf("bind failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("listen failed with error: %ld\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    printf("socket is listening at port %s...\n", DEFAULT_PORT);

    while (true)
    {
        // Create another SOCKET to handle client connection
        SOCKET accept_socket = INVALID_SOCKET;
        if ((accept_socket = accept(listen_socket, NULL, NULL)) == INVALID_SOCKET)
        {
            printf("failed to accept: %d\n", WSAGetLastError());
            closesocket(listen_socket);
            WSACleanup();
            return 1;
        }
        // Refill the buffer with DEFAULT_BUFLEN NULL characters
        char buffer[DEFAULT_BUFLEN] = {NULL};

        // Send data to the Client
        int req = recv(accept_socket, buffer, sizeof(buffer), 0);
        if (req == SOCKET_ERROR)
        {
            printf("failed to recv data: %ld\n", WSAGetLastError());
            return 1;
        }
        if (req == 0)
            printf("%ld\n", req);
        else
            printf("\n-> Received %d bytes, content:\n===\n%s\n===\n", req, buffer);

        // Hello, World!
        char *sendbuf = "HTTP/1.1 200 OK\nServer: HGanyu\nConnection: keep-alive\nContent-Length: 12\n\nHello, User!";
        int res = send(accept_socket, sendbuf, strlen(sendbuf), 0);
        if (res == SOCKET_ERROR)
        {
            printf("failed to send data: %ld\n", WSAGetLastError());
            return 1;
        }
        // Shutdown the connection
        iResult = shutdown(accept_socket, SD_SEND);
        if (iResult == SOCKET_ERROR)
        {
            printf("shutdown failed with error: %ld\n", WSAGetLastError());
            closesocket(accept_socket);
            WSACleanup();
            return 1;
        }
        closesocket(accept_socket);
    }

    _Log("WSAStartup() Closed");
    closesocket(listen_socket);
    WSACleanup();
    return 0;
}