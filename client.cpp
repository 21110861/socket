#include <iostream>
#include <string_view>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include<string>

constexpr int DEFAULT_BUFLEN = 512;
constexpr std::string_view DEFAULT_PORT = "27015";

SOCKET connectToServer(const char* const serverName) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    addrinfo* result = nullptr, * ptr = nullptr;
    addrinfo hints;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed with error: " << iResult << '\n';
        return INVALID_SOCKET;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(serverName, DEFAULT_PORT.data(), &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed with error: " << iResult << '\n';
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            std::cout << "socket failed with error: " << WSAGetLastError() << '\n';
            WSACleanup();
            return INVALID_SOCKET;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Unable to connect to server!\n";
        WSACleanup();
        return INVALID_SOCKET;
    }

    return ConnectSocket;
}

void sendData(const SOCKET ConnectSocket, const char* const sendbuf) {
    int iResult = send(ConnectSocket, sendbuf, static_cast<int>(strlen(sendbuf)), 0);
    if (iResult == SOCKET_ERROR) {
        std::cout << "send failed with error: " << WSAGetLastError() << '\n';
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }

    std::cout << "Bytes Sent: " << iResult << '\n';
}

void receiveData(const SOCKET ConnectSocket, char* const recvbuf, const int recvbuflen) {
    int iResult;

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            std::cout << "Bytes received: " << iResult << '\n';
        else if (iResult == 0)
            std::cout << "Connection closed\n";
        else
            std::cout << "recv failed with error: " << WSAGetLastError() << '\n';

}

void shutdownConnection(const SOCKET ConnectSocket) {
    int iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cout << "shutdown failed with error: " << WSAGetLastError() << '\n';
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " server-name\n";
        return 1;
    }

    SOCKET ConnectSocket = connectToServer(argv[1]);
    if (ConnectSocket == INVALID_SOCKET)
        return 1;

    

    char recvbuf[DEFAULT_BUFLEN];
    std::string message;
    while(message != "close"){
        std::cout<<"Nhập tin nhắn: ";
        std::getline(std::cin,message);
        sendData(ConnectSocket, message.c_str());
        receiveData(ConnectSocket, recvbuf, DEFAULT_BUFLEN);
    }
    shutdownConnection(ConnectSocket);

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
