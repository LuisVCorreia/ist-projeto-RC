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
#define DEFAULT_ASIP "127.0.0.1"

class ClientUDP {
    public:
        ClientUDP(const char* port, const char* asip);
        ~ClientUDP();
        void run();

    private:
        int fd;
        struct addrinfo *res;
        std::string asip;
        bool session_terminated = false;
        std::string uid;
        std::string password;
        
        void readCommand();
        void handleLogin(const std::string& additionalInfo);
        void handleLogout(const std::string& additionalInfo);
        void handleUnregister(const std::string& additionalInfo);
        void handleExit();
        int loginValid();
        void sendAuthMessage(std::string messageType);  
        void sendLoginMessage();
        void receiveLoginResponse();
        void sendLogoutMessage();
        void receiveLogoutResponse();
        void sendUnregisterMessage();
        void receiveUnregisterResponse();
};

#endif