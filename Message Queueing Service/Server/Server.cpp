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

#define BUFFER_SIZE 1024

Server::Server(const std::string& serverAddress, int port)
    : serverAddress(serverAddress), port(port), running(false){}

Server::~Server(){
    stop();
}


void Server::start() {
    running = true;

    clientThread = std::thread(&Server::handleClientConnection, this);      // Pokretanje niti za konekciju sa klijentom

    serverThread = std::thread(&Server::handleServerConnection, this);      // Pokretanje niti za konekciju sa serverom

    processMessages();      // Obrada poruka u glavnoj niti
}

void Server::stop() {
    running = false;


    // Obezbedjivanje da sve ostale niti prestanu sa izvrsavanjem
    if (clientThread.joinable())
        clientThread.join();

    if (serverThread.joinable())
        serverThread.join();

    std::cout << "Server stopped." << std::endl;
}

void Server::handleClientConnection() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Socket creation failed");
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

        char buffer[BUFFER_SIZE];
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            std::string message(buffer, bytesReceived);
            processingQueue.push(message);      // dodaje poruku u red za obradu                               TREBA IMPLEMENTIRATI
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();


}

void Server::handleServerConnection() {
    while (running) {
        // Ova nit šalje poruke drugom serveru koristeći sendingQueue
        while (!sendingQueue.empty()) {                                                                      //treba implementirati!!!!!!!!!!!!!!!
            std::string message = sendingQueue.pop();                                                        //treba implementirati!!!!!!!!!!!!!!!!!!
            std::cout << "Sending message to another server: " << message << std::endl;
            // Implementacija slanja poruka prema drugom serveru (TCP/UDP)
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Kratka pauza
    }
}




/*void handleClient(SOCKET clientSocket) {
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
}*/

int main() {
    const std::string serverAddress = "127.0.0.1"; // Adresa na kojoj server osluškuje
    const int serverPort = 8080;                  // Port na kojem server osluškuje

    try {
        // Kreiranje i pokretanje servera
        Server server(serverAddress, serverPort);
        std::cout << "Starting server..." << std::endl;
        server.start();

        // Program ostaje aktivan dok server radi
        std::cout << "Server is running. Press Enter to stop..." << std::endl;
        std::cin.get(); // Čeka unos korisnika za zaustavljanje servera

        server.stop(); // Zaustavlja server
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    std::cout << "Server has stopped." << std::endl;
    return 0;
}
