#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

void RunServerTCP()
{
    std::cout << "{I} Start TCP Server" << std::endl;
    //�������� ������ � ��������� ����������
    //������ ��������:
    //  AF_INET - IPv4 ���������
    //  AF_INET6 - IPv6 ���������
    //  AF_UNIX/AF_LOCAL - ��� ��������� ������� Unix, ���������� �� windows
    //���������� ������� � �������� ����������:
    //  SOCK_STREAM + IPPROTO_TCP = ����� ��� ������ � TCP �����������
    //  SOCK_DGRAM + IPPROTO_UDP = ����� ��� ������ � UDP �����������
    //  SOCK_RAW - ����� �������������� � TCP � UDP, ����� ����� �����
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) 
    {
        std::cerr << "[E] Failed create Socket." << std::endl;
        return;
    }

    //�������� ������ ������� � �����������:
    //sin_family = AF_INET - ��������� ��� ������������ IPv4, ������ sockaddr_in �������� ������ � IPv4
    //sin_addr.s_addr = htonl(INADDR_ANY) - ������� ��� ������ �������� � ���������� � ������� ������� ������
    //  ����� �������� �� ����� ��������� ��� ���������� ����� ����� inet_pton
    //sin_port = htons(7777) - ������������� ���� 7777, ������������� ���������� ����� �� little-endian � big-endian
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(7777); 
    
    //�������� ������ � ������ � �����
    //1: �������� ������
    //2: ��������� �� ��������� � �������
    //3: ������ ���������
    //bind �������� ��� � sockaddr_in ��� � � sockaddr_in6
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        std::cerr << "[E] Failed bind socket to addres and port." << std::endl;
        closesocket(serverSocket);
        return;
    }

    //��������� ������ � ����� �������������, � ��������� ������������ �������
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) 
    {
        std::cerr << "[E] Failed listen client." << std::endl;
        closesocket(serverSocket);
        return;
    }

    std::cout << "{I} TCP Server Start. Waiting client..." << std::endl;

    //�������� sockaddr_in ��� ������ �������
    //��������� ��� ���������� �� ������ serverSocket
    //�������� ������ ������ clientSocket ��� ����� � ��������
    //���� �� accept �� �������� ����� ����� ��� �������������� � ��������, �� ����� ���� �� ������������� ��������:
    //  SOCKET clientSocket= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //  bind(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), sizeof(clientAddr);
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) 
    {
        std::cerr << "[E] Failed conneting to client." << std::endl;
        closesocket(serverSocket);
        return;
    }

    std::cout << "{I} Connect client: " << inet_ntoa(clientAddr.sin_addr) << ":" << htons(clientAddr.sin_port) << std::endl;

    //����������� ������ � ����� � ������ clientSocket, � ����� buffer ������� sizeof(buffer)
    //���� 0 ������� � ��� ��� �� �������� ����� ������, � ������� ������ ��������� ��, � ������� �� �������
    //��� �� ����� ���� ������ �����:
    //  MSG_WAITALL - ����� ������� ���������� ������
    //  MSG_OOB  - ��������� ������� ������
    //  MSG_PEEK - ��������� ����� ��� ��� ������, �� �� ������� �������
    //  MSG_DONTWAIT - �� ����� ������ ���� �� ���
    char buffer[1024] = { 0 };
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived == SOCKET_ERROR) 
    {
        std::cerr << "[E] Error geting massage from client." << std::endl;
    }
    else 
    {
        std::cout << "{I} Massege from client: " << buffer << std::endl;

        //�������� ������
        //���� 0 ��� MSG_NOSIGNAL, ���������� ����� ��������
        //��� �� ����� ���� ������ �����:
        //  MSG_DONTROUTE - ������ �� ������ ������������������, � ������ ������������ �� ������ ����������
        //  MSG_OOB - �������� ������� ������
        //  MSG_WAITALL - ��������� ��
        const char* responseMessage = "������, ������!";
        send(clientSocket, responseMessage, strlen(responseMessage), 0);
    }

    //�������� �������
    closesocket(clientSocket);
    closesocket(serverSocket);
}

void RunClientTCP()
{
    std::cout << "{I} Start TCP Client" << std::endl;
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) 
    {
        std::cerr << "[E] Failed create Socket." << std::endl;
        return;
    }

    //����� � ������� �� serverAddr � RunServer() ����� ������� ������, � � ���� ����� ��������� ����� clientSocket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7777);
    inet_pton(AF_INET, "37.79.202.167", &serverAddr.sin_addr);

    //��������� ����������, � �������� ������ � �������
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        std::cerr << "[E] Failed connect to Server." << std::endl;
        closesocket(clientSocket);
        return;
    }

    const char* message = "������, ������!";
    send(clientSocket, message, strlen(message), 0);

    char buffer[1024] = { 0 };
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::cout << "{I} Massege from server: " << buffer << std::endl;

    closesocket(clientSocket);
}

void RunServerUDP()
{
    std::cout << "{I} Start UDP Server" << std::endl;
    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "[E] Failed create Socket." << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(7777);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "[E] Failed bind socket to address and port." << std::endl;
        closesocket(serverSocket);
        return;
    }

    std::cout << "{I} UDP Server Start. Waiting client..." << std::endl;

    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char buffer[1024] = { 0 };
    //��������� ��������� ��� �������� ������ � ������� ����������, � ��� �� ���������� ���������� �� �������
    int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
    if (bytesReceived == SOCKET_ERROR)
    {
        std::cerr << "[E] Error receiving message from client." << std::endl;
    }
    else
    {
        std::cout << "{I} Client: " << inet_ntoa(clientAddr.sin_addr) << ":" << htons(clientAddr.sin_port) << std::endl;
        std::cout << "{I} Message from client: " << buffer << std::endl;
        const char* responseMessage = "������, ������!";
        //�������� ��������� ��� �������� ������ � ������� ����������, � ��� �� ���������� ���������� �� �������
        sendto(serverSocket, responseMessage, strlen(responseMessage), 0, reinterpret_cast<sockaddr*>(&clientAddr), clientAddrSize);
    }
    closesocket(serverSocket);
}

void RunClientUDP()
{
    std::cout << "{I} Start UDP Client" << std::endl;
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "[E] Failed create Socket." << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7777);
    inet_pton(AF_INET, "37.79.202.167", &serverAddr.sin_addr);

    const char* message = "������, ������!";
    sendto(clientSocket, message, strlen(message), 0, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));

    char buffer[1024] = { 0 };
    sockaddr_in recvAddr;
    int recvAddrSize = sizeof(recvAddr);
    recvfrom(clientSocket, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&recvAddr), &recvAddrSize);
    std::cout << "{I} Message from server: " << buffer << std::endl;

    closesocket(clientSocket);
}

int main() 
{
    setlocale(LC_ALL, "Russian");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  //������������� Winsock, � ��������� ������ 2.2
    {
        std::cerr << "[E] Failed init Winsock." << std::endl;
        return 1;
    }
     
    int i = 1;
    std::cout << "(in) Number: ";
    std::cin >> i;

    if (i == 1)         RunServerTCP();
    else if (i > 1)     RunClientTCP();
    else if (i == -1)   RunServerUDP();
    else if (i < -1)    RunClientUDP();
    
    std::cin >> i;
    WSACleanup(); //�������� Winsock

    return 0;
}