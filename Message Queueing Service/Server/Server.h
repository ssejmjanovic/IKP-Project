#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <vector>
#include "../Common/MessageQueue.h"
#include "../Common/ConcreteMessageQueueServiceh.h"
#include "../Common/ThreadPool.h"

#pragma comment(lib, "ws2_32.lib")

class Server {
private:
    std::string serverAddress;      // Adresa servera
    int port;                       // Port servera
    std::atomic<bool> running;      // Indikator da li server radi
    ThreadPool threadPool;          // Thread pool za upravljanje nitima
    
    ConcreteMessageQueueService messageQueueService;


    MessageQueue<std::string> sendingQueue;     // Red za slanje poruka primljenih od klijenta
    MessageQueue<std::string> receivingQueue;  // Red za obradu primljenih poruka


    void handleClientConnection();                          // Metoda za rukovanje klijentom
    void handleServerConnection();                          // Metoda za rukovanje serverom
    void receiveFromOtherServer(SOCKET otherServerSocket);  // Metoda za primanje poruka od drugog servera
    void forwardToClient(SOCKET clientSocket);              // Metoda za prosledjivanje primljene poruke klijentu
    void processClientMessage(const std::string& message);  // Metoda za obradu poruke koju klijent posalje


public:
    Server(const std::string& serverAddress, int port, size_t threadPoolSize = 4);
    ~Server();

    // Metoda za slanje poruka u red
    void SendToQueue(const std::string& queueName, const std::string& message);
    
    // Start i stop servera
    void start();
    void stop();
};

#endif // SERVER_H
