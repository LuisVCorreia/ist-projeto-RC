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
        void receiveRequest();


    private:
        socklen_t client_addrlen;
        struct sockaddr_in client_addr;
        int socketUDP;

        void handleLogin(std::string& additionalInfo);
        void handleLogout(std::string& additionalInfo);
        void handleUnregister(std::string& additionalInfo);
        void handleAllAuctions(std::string& additionalInfo);
        void handleMyAuctions(std::string& additionalInfo);
        void handleMyBids(std::string& additionalInfo);
        // void handleShowRecord(std::string& additionalInfo);

        int sendResponse(const std::string& response);
       
};

#endif