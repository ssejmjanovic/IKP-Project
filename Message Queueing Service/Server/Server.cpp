#include "Server.h"
#include "../Common/MessageQueue.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") //Winsock biblioteka

/*
    Ako ne radi i izbacuje LINK greske, sledeci koraci resavaju problem:
    1. desni klik na Common -> Properties -> Configuration type = Static library, Output Directory = $(SolutionDir)Debug\
    2. desni klik na Server -> Properties -> Linker -> Input -> Additional Dependecies - dodati $(SolutionDir)Debug\Common.lib
    3. desni klik na Client -> Properties -> Linker -> Input -> Additional Dependecies - dodati $(SolutionDir)Debug\Common.lib
*/

#define BUFFER_SIZE 1024
#define OTHER_SERVER_PORT 8082
#define MY_SERVER_PORT 8080
#define MY_SERVER_IP "192.168.1.12"
#define OTHER_SERVER_IP "192.168.1.7"
#define IS_IT_CLIENT false
#define FRIEND "Soko"

Server::Server(const std::string& serverAddress, int port, bool isClient, size_t threadPoolSize)
    : serverAddress(serverAddress), port(port), running(false), isClient(isClient), threadPool(threadPoolSize){}

Server::~Server(){
    stop();
}


void Server::start() {
    running = true;

    // Pokretanje niti za rukovanje konekcijama preko ThreadPool-a
    if (isClient) {
        threadPool.enqueue([this]() {
            connectToOtherServer(OTHER_SERVER_IP, OTHER_SERVER_PORT);
            });
    }
    else {
        threadPool.enqueue([this]() {handleServerConnection(); });
    }
    
    threadPool.enqueue([this]() {handleClientConnection(); });
    

    std::cout << "Server started on " << serverAddress << ":" << port << std::endl;
}

void Server::stop() {
    running = false;
    
    std::cout << "Stopping server..." << std::endl;

    // Pauza kako bi se osigurao zavrsetak preostalih zadataka u ThreadPool-u
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Server stopped." << std::endl;
}

void Server::SendToQueue(const std::string& queueName, const std::string& message) {
    messageQueueService.SendMessageW(queueName.c_str(), message.data(), message.size());
}

void Server::receiveFromOtherServer(SOCKET otherServerSocket) {
    while (running) {
        char buffer[BUFFER_SIZE] = { 0 };

        int bytesReceived = recv(otherServerSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            std::string receivedMessage(buffer, bytesReceived);
            std::cout << "Received message from [" << FRIEND <<"]: " << receivedMessage << std::endl;

            receivingQueue.enqueue(receivedMessage);
        }
        else if (bytesReceived == 0) {
            std::cout << "Connection with other server closed." << std::endl;
            break;
        }
        else {
            std::cerr << "Error receiving message from other server." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Server::handleClientConnection() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error(" Socket creation failed");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error("Bind failed");
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error("Listen failed");
    }

    std::cout << "Listening for client connetions on port " << port << "..." << std::endl;

    while (running) {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            if (!running) break;
            std::cerr << "Accept failed" << WSAGetLastError() << std::endl;
            continue;
        }
        else
        {
            std::cout << "\nLocal client connected." << std::endl;
        }

        threadPool.enqueue([this, clientSocket = std::move(clientSocket)]() mutable {
            char buffer[BUFFER_SIZE];
            while (running) {
                int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesReceived > 0) {
                    std::string message(buffer, bytesReceived);
                    if (message == "exit") {
                        std::cout << "Client disconected." << std::endl;
                        break;
                    }
                    sendingQueue.enqueue(message);
                    std::cout << "Received message from client: " << message << std::endl;
                }
                else {
                    std::cout << "Error receiving data from client: " << WSAGetLastError() << std::endl;
                    break;
                }
            }
            closesocket(clientSocket);
        });

        threadPool.enqueue([this, clientSocket = std::move(clientSocket)]() mutable {
            while (running) {
                forwardToClient(clientSocket);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            closesocket(clientSocket);
        });
    }


    closesocket(serverSocket);
    WSACleanup();
}

void Server::handleServerConnection() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }


    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating server socket: " << WSAGetLastError() << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(OTHER_SERVER_PORT);     // Port drugog servera
    
    // Binds server socket
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error binding server socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        return;
    }

    // Postavljanje u rezim osluskivanja
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error setting server socket to listen mode: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        return;
    }

    std::cout << "Waiting for connection from another server on port " << port << "..." << std::endl;

    //Slanje poruka drugom serveru
    while (running) {
        
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET otherServerSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        char ipStr[INET_ADDRSTRLEN]; // Buffer za IPv4
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);

        std::cout << "Other Server IP: " << ipStr << std::endl;

        if (otherServerSocket == INVALID_SOCKET) {
            if (!running) break;
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }



        //PROBLEM OVDE
        // Pokretanje niti za primanje poruka od drugog servera
        threadPool.enqueue([this, otherServerSocket]() {receiveFromOtherServer(otherServerSocket); });

        // Pokratanje niti za slanje poruka drugom serveru
        threadPool.enqueue([this, otherServerSocket]() {
            while (running) {
                while (!sendingQueue.isEmpty()) {
                    std::string message = sendingQueue.dequeue();
                    int bytesSent = send(otherServerSocket, message.c_str(), message.size(), 0);
                    if (bytesSent == SOCKET_ERROR) {
                        std::cerr << "Error sending message to server: " << WSAGetLastError() << std::endl;
                        break;
                    }
                    std::cout << "Sent message to server: " << message << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            closesocket(otherServerSocket);

            });
    }

    closesocket(serverSocket);
    
}

void Server::connectToOtherServer(const std::string& otherServerIp, int otherServerPort) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed.");
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating client socket: " << WSAGetLastError() << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, otherServerIp.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(otherServerPort);

    std::cout << "Connecting to other server at " << otherServerIp << ":" << otherServerPort << "..." << std::endl;

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to other server: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    std::cout << "Connected to other server!" << std::endl;

    threadPool.enqueue([this, clientSocket]() {receiveFromOtherServer(clientSocket); });

    threadPool.enqueue([this, clientSocket]() {
        while (running) {
            while (!sendingQueue.isEmpty()) {
                std::string message = sendingQueue.dequeue();
                int bytesSent = send(clientSocket, message.c_str(), message.size(), 0);
                if (bytesSent == SOCKET_ERROR) {
                    std::cerr << "Error sending message to other server: " << WSAGetLastError() << std::endl;
                    break;
                }
                std::cout << "Sent message to other server: " << message << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        closesocket(clientSocket);
        });
}

void Server::forwardToClient(SOCKET clientSocket) {
    while (!receivingQueue.isEmpty()) {
        std::string message = receivingQueue.dequeue();
        int bytesSent = send(clientSocket, message.c_str(), message.size(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error sending message to client: " << WSAGetLastError() << std::endl;
            return;
        }
        std::cout << "Forwarded message to client: " << message << std::endl;
    }
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//---------------------            MAIN             --------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------


int main() {
    const std::string serverAddress = MY_SERVER_IP;                  // IP adresa mog servera
    const int serverPort = MY_SERVER_PORT;                           // Port na kojem server osluskuje za svog klijenta
    const std::string otherServerIp = OTHER_SERVER_IP;               // IP adresa drugog racunara
    const int otherServerPort = OTHER_SERVER_PORT;                   // Port za komunikaciju izmedju servera

    try {
        // Kreiranje i pokretanje servera
        Server server(serverAddress, serverPort, IS_IT_CLIENT);
        std::cout << "Starting server..." << serverAddress << ":" << serverPort << "..." << std::endl;
        server.start();

        // Program ostaje aktivan dok server radi
        std::cout << "Server is running. Type 'exit' and press Enter t stop..." << std::endl;
        std::string command;
        while (true) {
            std::getline(std::cin, command);
            if (command == "exit") {
                std::cout << "Stopping server..." << std::endl;
                break;
            }
            else {
                std::cout << "Unknown command. Type 'exit' to stop the server." << std::endl;
            }
        }

        server.stop(); // Zaustavlja server
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    std::cout << "Server has stopped." << std::endl;
    return 0;
}
