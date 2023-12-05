#ifndef CLIENTTCP_H
#define CLIENTTCP_H

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
#include <signal.h>

#define HOST "tejo.tecnico.ulisboa.pt"
#define DEFAULT_PORT "58047"
#define DEFAULT_ASIP "127.0.0.1"

class ClientTCP {
    public:
        ClientTCP(const char* port, const char* asip);
        ~ClientTCP();
        void handleTest();
    
    private:
        int fd;
        struct addrinfo *res;
        std::string asip;
};

#endif