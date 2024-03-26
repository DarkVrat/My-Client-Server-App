#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

void RunServerTCP()
{
    std::cout << "{I} Start TCP Server" << std::endl;
    //Создание сокета с указанием параметров
    //Первый параметр:
    //  AF_INET - IPv4 протоколы
    //  AF_INET6 - IPv6 протоколы
    //  AF_UNIX/AF_LOCAL - для локальных сокетов Unix, недоступны на windows
    //комбинации второго и третьего параметров:
    //  SOCK_STREAM + IPPROTO_TCP = Сокет для работы с TCP протоколами
    //  SOCK_DGRAM + IPPROTO_UDP = Сокет для работы с UDP протоколами
    //  SOCK_RAW - может использоваться с TCP и UDP, более сырой сокет
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) 
    {
        std::cerr << "[E] Failed create Socket." << std::endl;
        return;
    }

    //Создание адреса сервера с параметрами:
    //sin_family = AF_INET - уточнение что используется IPv4, однако sockaddr_in работает только с IPv4
    //sin_addr.s_addr = htonl(INADDR_ANY) - говорим что сервер работает с локальными и внешним адресом машины
    //  можно изменить на число локальный или глобальный адрес через inet_pton
    //sin_port = htons(7777) - устанавливаем порт 7777, дополнительно преобразуя число из little-endian в big-endian
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(7777); 
    
    //Привязка сокета к адресу и порту
    //1: передача сокета
    //2: указатель на структуру с адресом
    //3: размер структуры
    //bind работает как с sockaddr_in так и с sockaddr_in6
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        std::cerr << "[E] Failed bind socket to addres and port." << std::endl;
        closesocket(serverSocket);
        return;
    }

    //Установка сокета в режим прослушивания, и установка максимальной очереди
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) 
    {
        std::cerr << "[E] Failed listen client." << std::endl;
        closesocket(serverSocket);
        return;
    }

    std::cout << "{I} TCP Server Start. Waiting client..." << std::endl;

    //создание sockaddr_in для адреса клиента
    //Получение его параметров по сокету serverSocket
    //создание нового сокета clientSocket для связи с клиентом
    //Если бы accept не создавал новый сокет для взаимодействия с клиентом, то нужно было бы дополнительно написать:
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

    //считываение данных в буфер с сокета clientSocket, в бефер buffer размера sizeof(buffer)
    //Флаг 0 говорит о том что бы операция ждала данные, и получив данные сохранила их, и удалила из очереди
    //Так же могут быть другие флаги:
    //  MSG_WAITALL - ждать полного заполнения буфера
    //  MSG_OOB  - получение срочных данных
    //  MSG_PEEK - заполнить буфер тем что пришло, но не очищать очередь
    //  MSG_DONTWAIT - не ждать данные если их нет
    char buffer[1024] = { 0 };
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived == SOCKET_ERROR) 
    {
        std::cerr << "[E] Error geting massage from client." << std::endl;
    }
    else 
    {
        std::cout << "{I} Massege from client: " << buffer << std::endl;

        //Отправка данных
        //Флаг 0 или MSG_NOSIGNAL, отсутствие спецю действий
        //Так же могут быть другие флаги:
        //  MSG_DONTROUTE - данные не должны маршрутизироваться, и только отправляться на нужное устройство
        //  MSG_OOB - отправка срочных данных
        //  MSG_WAITALL - отправить всё
        const char* responseMessage = "Привет, клиент!";
        send(clientSocket, responseMessage, strlen(responseMessage), 0);
    }

    //Закрытие сокетов
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

    //Здесь в отличие от serverAddr в RunServer() адрес сервера указан, и к нему бодет подключён сокет clientSocket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7777);
    inet_pton(AF_INET, "37.79.202.167", &serverAddr.sin_addr);

    //Установка соединения, и передача данных о клиенте
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        std::cerr << "[E] Failed connect to Server." << std::endl;
        closesocket(clientSocket);
        return;
    }

    const char* message = "Привет, сервер!";
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
    //Получение сообщения без создания сокета и прямого соединения, а так же заполнение информации об клиенте
    int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
    if (bytesReceived == SOCKET_ERROR)
    {
        std::cerr << "[E] Error receiving message from client." << std::endl;
    }
    else
    {
        std::cout << "{I} Client: " << inet_ntoa(clientAddr.sin_addr) << ":" << htons(clientAddr.sin_port) << std::endl;
        std::cout << "{I} Message from client: " << buffer << std::endl;
        const char* responseMessage = "Привет, клиент!";
        //Отправка сообщения без создания сокета и прямого соединения, а так же заполнение информации об клиенте
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

    const char* message = "Привет, сервер!";
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
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  //Инициализация Winsock, с указанием версии 2.2
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
    WSACleanup(); //Закрытие Winsock

    return 0;
}