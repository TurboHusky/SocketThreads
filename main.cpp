#include <sys/socket.h>
#include <netinet/in.h> // Input struct for sockets
#include <arpa/inet.h>  // Helper functions for ip conversion
#include <thread>
#include <iostream>

int client_app(sockaddr_in server_addr)
{
    int client;
    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0)
    {
        std::cout << "Client: Failed to open socket." << std::endl;
        return 1;
    }
    std::cout << "Client: Connecting...\n";
    if (connect(client, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cout << "Client: Failed to connect - " << strerror(errno) << std::endl;
        return 1;
    }
    char send_buffer[65536]{"Client message."};
    std::cout << "Client: Sending...\n";
    send(client, send_buffer, strlen(send_buffer), 0);
    int reader;
    char buffer[1024]{0};
    reader = recv(client, &buffer, 1024, MSG_WAITFORONE);
    std::cout << "Client: Read " << reader << " bytes: \"" << buffer << "\"\n";

    std::cout << "Client: Cleaning up." << std::endl;
    shutdown(client, SHUT_RDWR);

    return 0;
}

int server_app(sockaddr_in server_addr)
{
    int server;
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
    {
        std::cout << "Server: Failed to open socket." << std::endl;
        return 1;
    }
    int opted{1};
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opted, sizeof(opted)))
    { // Set option at socket level to let bind reuse local addresses.
        std::cout << "Server: Failed to set socket options - " << strerror(errno) << std::endl;
        return 1;
    }
    // Bind
    std::cout << "Server: Binding...\n";
    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cout << "Sever: Failed to bind to address - " << strerror(errno) << "\n";
        return 1;
    }
    // Listen
    std::cout << "Server: Listening...\n";
    if (listen(server, 3) < 0)
    {
        std::cout << "Server: Failed to start listening - " << strerror(errno) << std::endl;
        return 1;
    }

    // Accept
    int conn;
    socklen_t server_length = sizeof(server_addr);
    std::cout << "Server: Accept connection...\n";
    conn = accept(server, (struct sockaddr *)&server_addr, &server_length);
    if (conn < 0)
    {
        std::cout << "Server: Failed to accept connection." << std::endl;
        return 1;
    }

    int reader;
    char buffer[1024]{0};
    reader = recv(conn, &buffer, 1024, MSG_WAITFORONE);
    // reader = read(conn, &buffer, 1024);
    std::cout << "Server: Read " << reader << " bytes: \"" << buffer << "\"\n";
    char msg[]{"Server response."};
    send(conn, msg, strlen(msg), 0);
    std::cout << "Server: Sent response to client.\n";

    std::cout << "Server: Cleaning up.\n";
    shutdown(conn, SHUT_RDWR);
    shutdown(server, SHUT_RDWR);

    return 0;
}

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv; // Ignore unused input variables

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9977); // Set port to network order

    std::thread t1(server_app, server_addr);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr) <= 0)
    {
        std::cout << "Failed to convert IP to binary format." << std::endl;
        return 1;
    }
    // server_addr.sin_addr.s_addr = INADDR_LOOPBACK;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::thread t2(client_app, server_addr);
    t1.join();
    t2.join();

    return 0;
}