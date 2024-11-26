#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") //Winsock biblioteka

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void error(const char* msg) {
    std::cerr << msg << ": " << WSAGetLastError() << std::endl;
    exit(EXIT_FAILURE);
}

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        // Primanje poruka od klijenta
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        std::cout << "Received from client: " << buffer << std::endl;

        // Echo poruka za proveru da li funkcionise
        std::string response = "Echo: " + std::string(buffer);
        send(clientSocket, response.c_str(), response.size(), 0);
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;

    // Inicijalizacija Winsock-a
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error("WSAStartup failed");
    }

    //Kreiranje server socket-a
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        error("Socket creation failed");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    //Bind server socket-a
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        error("Bind failed");
    }

    //Osluskivanje radi konekcije
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        error("Listen failed");
    }

    std::cout << "Server is listening on port " << SERVER_PORT << std::endl;

    // Petlja za prihvatanje klijenata
    while (true) {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue; // Nastavlja slušanje novih klijenata
        }

        char clientIp[INET_ADDRSTRLEN]; // Bafer za IP adresu klijenta
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN); // Konvertovanje adrese u čitljiv format
        std::cout << "Client connected: " << clientIp << std::endl;

        // Rukovanje komunikacijom sa klijentom
        handleClient(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
