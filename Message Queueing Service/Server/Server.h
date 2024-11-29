#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <vector>
#include "../Common/MessageQueue.h"
#include "../Common/ConcreteMessageQueueServiceh.h"

#pragma comment(lib, "ws2_32.lib")

class Server {
private:
    std::string serverAddress;      // Adresa servera
    int port;                       // Port servera
    bool running;                   // Indikator da li server radi
    std::thread clientThread;       // Nit za komunikaciju sa klijentom
    std::thread serverThread;       // Nit za komunikaciju sa drugim serverom
    
    ConcreteMessageQueueService messageQueueService;


    MessageQueue<std::string> sendingQueue;     // Red za slanje poruka primljenih od klijenta
    MessageQueue<std::string> receivingQueue;  // Red za obradu primljenih poruka


    void handleClientConnection();       // Metoda za rukovanje klijentom
    void handleServerConnection();       // Metoda za rukovanje serverom
    void processMessages();              // Metoda za obradu poruka

public:
    Server(const std::string& serverAddress, int port);
    ~Server();


    void SendToQueue(const std::string& queueName, const std::string& message);
    void start();
    void stop();
};

#endif // SERVER_H
