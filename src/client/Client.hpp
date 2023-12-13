#ifndef CLIENT_H
#define CLIENT_H

#include "clientUDP.hpp"
#include "clientTCP.hpp"


class Client {
    public:
        Client(const char* port, const char* asip);
        void run();

        static void setCurrentClientInstance(Client* instance);
        static void sigintHandler(int signum);
        

    private:
        static Client* currentInstance;
        ClientUDP clientUDP;
        ClientTCP clientTCP;
        bool session_terminated = false;
        std::string uid;
        std::string password;
        void readCommand();
        void handleExit();
        void logoutIfLoggedIn();
};


#endif