#undef UNICODE

#include <iostream>
#include <string_view>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <vector>

constexpr int DEFAULT_BUFLEN = 512;
constexpr std::string_view DEFAULT_PORT = "27015";

SOCKET setupServerSocket()
{
    WSADATA wsaData;
    int iResult;
    SOCKET ListenSocket = INVALID_SOCKET;
    struct addrinfo *result = nullptr;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        std::cerr << "WSAStartup failed with error: " << iResult << '\n';
        return INVALID_SOCKET;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT.data(), &hints, &result);
    if (iResult != 0)
    {
        std::cerr << "getaddrinfo failed with error: " << iResult << '\n';
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if (iResult == SOCKET_ERROR)
    {
        std::cerr << "bind failed with error: " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        std::cerr << "listen failed with error: " << WSAGetLastError() << '\n';
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // if (SetSocketNonBlocking(ListenSocket) == SOCKET_ERROR) {
    //     printf("Error setting socket to non-blocking mode: %d\n", WSAGetLastError());
    //     closesocket(ListenSocket);
    //     WSACleanup();
    //     return INVALID_SOCKET;
    // }

    return ListenSocket;
}

void handleClient(SOCKET ClientSocket)
{
    int iResult;
    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN]{};
    int recvbuflen = DEFAULT_BUFLEN;

    do
    {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        std::cout<<"iresult= "<<iResult<<'\n';
        if (iResult > 0)
        {
            size_t len = strlen(recvbuf);
            std::cout << "Bytes received: " << iResult << len << "\ntext=" << recvbuf<<'\n';

            if (strcmp(recvbuf, "close") == 0)
            {
                break;
            }

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR)
            {
                std::cerr << "send failed with error: " << WSAGetLastError() << '\n';
                closesocket(ClientSocket);
                return;
            }
            std::cout << "Bytes sent: " << iSendResult << '\n';
            memset(recvbuf, 0, len);
        }
        else if (iResult == 0)
        {
            std::cout << "Connection closing...\n";
        }
        else
        {
            std::cerr << "recv failed with error: " << WSAGetLastError() << '\n';
            closesocket(ClientSocket);
            return;
        }
        // Check if received string is "close"
    } while (iResult > 0);
    std::puts("shutdown");
    // Shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        std::cerr << "shutdown failed with error: " << WSAGetLastError() << '\n';
    }

    // Cleanup
    closesocket(ClientSocket);
    return;
}

int main(int argc,char* argv[])
{
    size_t maxConnect = 1;
    if( argc > 1){
        maxConnect = std::stoull(argv[1]);
    }
    SOCKET ListenSocket = setupServerSocket();
    if (ListenSocket == INVALID_SOCKET)
        return 1;

    std::vector<std::thread> threads;
    threads.reserve(maxConnect);
    do
    {
        // Accept a client socket
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);

        if (ClientSocket == INVALID_SOCKET)
        {
            break;
            // Handle client in a separate thread
        }
        threads.emplace_back(handleClient, ClientSocket);

    } while (threads.size() < maxConnect);

    std::cout << "không nhận thêm kết nối, số kết nối: " << threads.size() << "\n";


    // Wait for all threads to finish
    for (auto &thread : threads)
    {
        thread.join();
    }
    // Close the listening socket
    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}
