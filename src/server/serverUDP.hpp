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

class ServerUDP {
    public:
        ServerUDP(const char* port, int& udp_socket);

    private:
        struct addrinfo hints, *res;
       
};

#endif