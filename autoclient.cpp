#include <iostream>
#include <string_view>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <memory>

constexpr int DEFAULT_BUFLEN = 512;
constexpr std::string_view DEFAULT_PORT = "27015";
enum class MY_ERROR : int
{
    SUCCESS,
    InitError,
    HostNameError,
    AddressError,
    IPError,
};
MY_ERROR InitWinsock()
{
    WSADATA wsaData;

    /*Initialize Winsock*/
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        std::cout << "WSAStartup failed with error: " << iResult << '\n';
        return MY_ERROR::InitError;
    }
    return MY_ERROR::SUCCESS;
}

std::string GetHostName()
{
    char hostname[256]{};
    int result = gethostname(hostname, sizeof(hostname));
    if (result == SOCKET_ERROR)
    {
        std::cerr << "gethostname failed: " << WSAGetLastError() << std::endl;
        return "";
    }
    return std::string(hostname);
}

std::string GetHostIP(const char *const hostname)
{

    addrinfo *addrInfo = nullptr;
    addrinfo hints{};
    hints.ai_family = AF_INET;

    if (getaddrinfo(hostname, nullptr, &hints, &addrInfo) != 0)
    {
        std::cerr << "getaddrinfo failed" << std::endl;
        return "";
    }

    sockaddr_in *ipAddress = reinterpret_cast<sockaddr_in *>(addrInfo->ai_addr);
    char ipString[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ipAddress->sin_addr), ipString, INET_ADDRSTRLEN);
    freeaddrinfo(addrInfo);

    return ipString;
}

SOCKET connectToServer(const char *const serverName)
{
    SOCKET ConnectSocket = INVALID_SOCKET;
    addrinfo *result = nullptr, *ptr = nullptr;
    addrinfo hints{};
    int iResult;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    /*Resolve the server address and port*/
    iResult = getaddrinfo(serverName, DEFAULT_PORT.data(), &hints, &result);
    if (iResult != 0)
    {
        std::cout << "getaddrinfo failed with error: " << iResult << '\n';
        WSACleanup();
        return INVALID_SOCKET;
    }

    /*Attempt to connect to an address until one succeeds*/
    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
    {
        /* Create a SOCKET for connecting to server*/
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            std::cout << "socket failed with error: " << WSAGetLastError() << '\n';
            WSACleanup();
            return INVALID_SOCKET;
        }

        /*Connect to server.*/
        iResult = connect(ConnectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        std::cout << "Unable to connect to server!\n";
        WSACleanup();
        return INVALID_SOCKET;
    }

    return ConnectSocket;
}

void sendData(const SOCKET ConnectSocket, const char *const sendbuf)
{
    int iResult = send(ConnectSocket, sendbuf, static_cast<int>(strlen(sendbuf)), 0);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "send failed with error: " << WSAGetLastError() << '\n';
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }

    std::cout << "Bytes Sent: " << iResult << '\n';
}

void receiveData(const SOCKET ConnectSocket, char *const recvbuf, const int recvbuflen)
{

    int iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0)
        std::cout << "Bytes received: " << iResult << '\n';
    else if (iResult == 0)
        std::cout << "Connection closed\n";
    else
        std::cout << "recv failed with error: " << WSAGetLastError() << '\n';
}

void shutdownConnection(const SOCKET ConnectSocket)
{
    int iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "shutdown failed with error: " << WSAGetLastError() << '\n';
        return;
    }
    closesocket(ConnectSocket);
}

void CommunicateWithServer(const char *const ip)
{
    SOCKET ConnectSocket = connectToServer(ip);
    if (ConnectSocket == INVALID_SOCKET)
        return;

    char recvbuf[DEFAULT_BUFLEN];
    std::string message;
    while (message != "close")
    {
        std::cout << "Nhập tin nhắn: ";
        std::getline(std::cin, message);
        if (message.empty() == false)
        {
            sendData(ConnectSocket, message.c_str());
            receiveData(ConnectSocket, recvbuf, DEFAULT_BUFLEN);
        }
    }

    shutdownConnection(ConnectSocket);
}

void MakeConnection()
{
    std::string hostname = GetHostName();
    if (hostname.empty())
    {
        std::puts("get hostname fail");
        return;
    }
    std::string ip = GetHostIP(hostname.c_str());
    if (ip.empty())
    {
        std::puts("ip fail");
        return;
    }
    CommunicateWithServer(ip.c_str());
}

int FailMessage(const char *const message)
{
    std::puts(message);
    return 1;
}

int main()
{
    if (InitWinsock() != MY_ERROR::SUCCESS)
    {
        MakeConnection();
        WSACleanup();
    }

    return 0;
}
