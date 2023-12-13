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

class ServerUDP {
    public:
        ServerUDP(const char* port, int& socketUDP);
        void receiveRequest(int& socketUDP);


    private:
        struct addrinfo hints, *res;
        void handleLogin(std::string& additionalInfo);
       
};

#endif