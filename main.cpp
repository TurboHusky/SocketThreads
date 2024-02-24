#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <thread>

int client_app(sockaddr_in server_addr)
{
    (void)server_addr;
    struct addrinfo *result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo("127.0.0.1", "9977", &hints, &result) != 0)
    {
        printf("Failed to get address\n");
        return -1;
    }
    SOCKET client;
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET)
    {
        printf("Client: Failed to open socket.");
        return 1;
    }
    printf("Client: Connecting...\n");
    if (connect(client, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
    {
        printf("Client: Failed to connect [%d]\n", WSAGetLastError());
        return 1;
    }
    char send_buffer[65536]{"Client message."};
    printf("Client: Sending...\n");
    send(client, send_buffer, strlen(send_buffer), 0);
    int reader;
    char buffer[1024]{0};
    reader = recv(client, &buffer[0], 1024, 0);
    printf("Client: Read %d bytes: \"%s\"\n", reader, buffer);

    printf("Client: Cleaning up.\n");
    closesocket(client);
    freeaddrinfo(result);
    return 0;
}

int server_app(sockaddr_in server_addr)
{
    (void)server_addr;
    struct addrinfo *result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, "9977", &hints, &result) != 0)
    {
        printf("Failed to get address\n");
        return -1;
    }

    SOCKET server{INVALID_SOCKET};
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server == INVALID_SOCKET)
    {
        int winerr = WSAGetLastError();
        printf("Server: Failed to open socket [%d]\n", winerr);
        return -1;
    }
    const char opted{1};
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opted, sizeof(opted)))
    {
        printf("Server: Failed to set socket options [%d]\n", WSAGetLastError());
        return 1;
    }
    // Bind
    printf("Server: Binding...\n");

    if (bind(server, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
    {
        printf("Sever: Failed to bind to address [%d]\n", WSAGetLastError());
        return 1;
    }
    // Listen
    printf("Server: Listening...\n");
    if (listen(server, 3) == SOCKET_ERROR)
    {
        printf("Server: Failed to start listening [%d]\n", WSAGetLastError());
        return 1;
    }

    // Accept
    SOCKET conn;
    printf("Server: Accept connection...\n");
    conn = accept(server, NULL, NULL);
    if (conn == INVALID_SOCKET)
    {
        printf("Server: Failed to accept connection\n");
        return 1;
    }

    int reader;
    char buffer[1024]{0};
    printf("Server: Receiving...\n");
    reader = recv(conn, &buffer[0], 1024, 0);
    printf("Server: Read %d bytes: \"%s\"\n", reader, buffer);
    char msg[]{"Server response."};
    send(conn, msg, strlen(msg), 0);
    printf("Server: Sent response to client.\n");

    printf("Server: Cleaning up\n");
    closesocket(server);
    freeaddrinfo(result);
    return 0;
}

DWORD WINAPI threadtest(LPVOID threadparams)
{
    (void)threadparams;
    printf("Thread called...\n");
    return 0;
}

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv; // Ignore unused input variables

    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("Unable to initialise Windows socket system: %d\n", iResult);
        return 1;
    }

    printf("Starting thread\n");
    DWORD id1{0};
    HANDLE winthread = CreateThread(NULL, 0, threadtest, NULL, 0, &id1);
    if (id1 == 0 || winthread == NULL)
    {
        printf("Error crating thread\n");
        return -1;
    }
    // system("pause"); // Win specific -> "Press any key to continue"
    Sleep(1000);
    printf("Thread Id: %lu\n", id1);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9977); // Set port to network order

    std::thread t1(server_app, server_addr);
    std::string ip{"127.0.0.1"};

    if (InetPton(AF_INET, ip.c_str(), &server_addr.sin_addr.s_addr) == SOCKET_ERROR)
    {
        printf("Failed to convert IP to binary format.");
        return 1;
    }

    server_addr.sin_addr.s_addr = INADDR_LOOPBACK;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::thread t2(client_app, server_addr);
    t1.join();
    t2.join();

    WSACleanup();

    return 0;
}