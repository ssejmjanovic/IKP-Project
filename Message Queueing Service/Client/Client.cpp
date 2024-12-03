#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <chrono>
#include "../Common/ThreadPool.h"

#pragma comment(lib, "ws2_32.lib") // Linkovanje Winsock biblioteke

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define FRIEND "Soko"

std::string globalShutdownFlag = "running";

void error(const char* msg) {
    std::cerr << msg << ": " << WSAGetLastError() << std::endl;
    exit(EXIT_FAILURE);
}

void receiveMessages(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    while (globalShutdownFlag == "running") {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "\nFailed to receive message from server." << std::endl;
            break;
        }
        if (bytesReceived == 0) {
            continue;
        }
        buffer[bytesReceived] = '\0';
        std::cout << "\n[" << FRIEND <<"]: " << buffer << std::endl;
        std::cout << "(type 'exit' to quit) [Me]: ";
    }
}

void sendMessage(SOCKET clientSocket) {
    std::string message;
    while(globalShutdownFlag == "running") {
        std::cout << "\n(type 'exit' to quit) [Me]: ";
        std::getline(std::cin, message);
        if (message == "exit") {
            send(clientSocket, message.c_str(), message.size(), 0);
            globalShutdownFlag = "shutdown";
            break;
        }
        
        if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
            std::cerr << "\nFailed to send message to server. " << std::endl;
            continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
}

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error("WSAStartup failed");
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        error("Socket creation failed");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);

    // OVDE IDE IP ADRESA RACUNARA NA KOM JE POKRENUTO lokalna adresa
    inet_pton(AF_INET, "192.168.1.12", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        error("Connection to server failed");
    }

    std::cout << "Connected to local server." << std::endl;

    ThreadPool threadPool(2);
    threadPool.enqueue([clientSocket]() {
        receiveMessages(clientSocket);
        });
    threadPool.enqueue([clientSocket]() {
        sendMessage(clientSocket);
        });

    while (globalShutdownFlag == "running") {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    

    closesocket(clientSocket);
    WSACleanup();

    std::cout << "Client has stopped." << std::endl;
    return 0;
}
