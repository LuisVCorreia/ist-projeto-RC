#ifndef SERVERTCP_H
#define SERVERTCP_H

#include <cstring>    
#include <cstdlib>      
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>     
#include <unistd.h>     
#include <cstdio>

class ServerTCP {
    public:
        ServerTCP(const char* port, int& tcp_socket);

    private:
        struct addrinfo hints, *res;
       
};

#endif