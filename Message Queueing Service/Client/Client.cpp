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

void communicateWithServer(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    std::string message;

    while (true) {
        std::cout << "Enter message to send to server (type 'exit' to quit): ";
        std::getline(std::cin, message);

        if (message == "exit") {
            std::cout << "Exiting communication..." << std::endl;
            break;
        }

        // Slanje poruke serveru
        if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
            std::cerr << "Send failed. Reconnecting..." << std::endl;
            break;
        }

        // Primanje odgovora od servera
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Server disconnected. Reconnecting..." << std::endl;
            break;
        }

        std::cout << "Server response: " << std::string(buffer, bytesReceived) << std::endl;
    }
}

int main() {
    WSADATA wsaData;

    // Inicijalizacija Winsock-a
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error("WSAStartup failed");
    }

    const char* serverIP = "127.20.10.6"; // Zameni sa IP adresom servera

    while (true) {
        // Kreiranje socket-a
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            error("Socket creation failed");
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SERVER_PORT);
        if (inet_pton(AF_INET, serverIP, &serverAddr.sin_addr) <= 0) {
            error("Invalid address/Address not supported");
        }

        // Pokušaj povezivanja na server
        std::cout << "Connecting to server at " << serverIP << ":" << SERVER_PORT << "..." << std::endl;
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed. Retrying in 5 seconds..." << std::endl;
            closesocket(clientSocket);
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Čeka 5 sekundi pre ponovnog pokušaja
            continue;
        }

        std::cout << "Connected to server." << std::endl;

        // Komunikacija sa serverom
        communicateWithServer(clientSocket);

        // Zatvaranje socket-a nakon prekida veze
        closesocket(clientSocket);

        std::cout << "Reconnecting to server..." << std::endl;

        //rsaaaaaaaaa
    }

    WSACleanup();
    return 0;
}
