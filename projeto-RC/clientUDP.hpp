#ifndef CLIENTUDP_H
#define CLIENTUDP_H

#include <iostream>
#include <string>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <algorithm>
#include <sstream>

#define HOST "tejo.tecnico.ulisboa.pt"
#define DEFAULT_PORT "58047"

class ClientUDP {
    public:
        ClientUDP(const std::string& port, bool verbose);
        ~ClientUDP();
        void run();

    private:
        int fd;
        struct addrinfo *res;
        bool verbose;
        bool session_terminated = false;
        std::string uid;
        std::string password;
        
        void readCommand();
        void handleLogin(const std::string& additionalInfo);
        void handleLogout(const std::string& additionalInfo);
        void handleExit();
        int loginValid();
        void sendLoginMessage();
        void receiveLoginResponse();
        void sendLogoutMessage();
        void receiveLogoutResponse();
        void sendAuthMessage(std::string messageType);  
};

#endif