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
#include <string>
#include <iostream>
#include <algorithm>
#include <utils.hpp>


class ServerTCP {
    public:
        ServerTCP(const char* port, int& socketTCP);
        void receiveRequest();

    private:
        int socketTCP;
        struct addrinfo hints, *res;
       
};

#endif