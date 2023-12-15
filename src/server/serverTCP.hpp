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
#include <common/common.hpp>


class ServerTCP {
    public:
        ServerTCP(const char* port, int& socketTCP);
        void receiveRequest(int& as_socket);

    private:
        int socketTCP;
        struct addrinfo hints, *res;
        int auctionCounter;

        struct OpenRequestInfo {
            std::string uid;
            std::string password;
            std::string name;
            std::string start_value;
            std::string timeactive;
            std::string fname;
            std::string fsize;
            std::string fdata; 
        };
        
        void handleOpen(std::string& additionalInfo);
        void handleClose(std::string& additionalInfo);

        int parseOpenRequestInfo(std::string& additionalInfo, OpenRequestInfo& openRequestInfo);
        int validateOpenRequestInfo(OpenRequestInfo& openRequestInfo);

        int sendResponse(const char* response);
};

#endif