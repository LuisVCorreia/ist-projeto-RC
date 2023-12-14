#ifndef SERVERUDP_H
#define SERVERUDP_H

#include <cstring>    
#include <cstdlib>      
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>     
#include <unistd.h>     
#include <cstdio>
#include <string>
#include <iostream>
#include <algorithm>

#include <utils.hpp>
#include <common/common.hpp>

class ServerUDP {
    public:
        ServerUDP(const char* port, int& socketUDP);
        void receiveRequest(int& socketUDP);


    private:
        struct addrinfo *res;
        int socketUDP;
        void handleLogin(std::string& additionalInfo);
        void handleLogout(std::string& additionalInfo);

        void sendResponse(const char* response);
       
};

#endif