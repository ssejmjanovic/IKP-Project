#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib") // Linkovanje Winsock biblioteke

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024


void error(const char* msg) {
    std::cerr << msg << ": " << WSAGetLastError() << std::endl;
    exit(EXIT_FAILURE);
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
    // OVDE IDE IP ADRESA RACUNARA NA KOM JE POKRENUTO
    inet_pton(AF_INET, "192.168.1.43", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        error("Connection to server failed");
    }

    std::cout << "Connected to server" << std::endl;

    while (true) {
        std::string message, queueName;
        std::cout << "Enter queue name: ";
        std::getline(std::cin, queueName);
        std::cout << "Enter message: ";
        std::getline(std::cin, message);

        if (queueName == "exit" || message == "exit") {
            break;
        }

        // Kombinujemo queueName i message za slanje serveru
        std::string combinedMessage = queueName + ":" + message;
        if (send(clientSocket, combinedMessage.c_str(), combinedMessage.size(), 0) == SOCKET_ERROR) {
            std::cerr << "Failed to send message to server." << std::endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
