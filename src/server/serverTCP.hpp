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
#include <thread>
#include <mutex>

#include <utils.hpp>
#include <common/common.hpp>

#define TCP_SERVER_TIMEOUT 5

class ServerTCP {
    public:
        ServerTCP(const char* port, int& socketTCP, int verbose);
        ~ServerTCP();
        void handleTCP();
        void handleClient(int client_socket);
        int acceptClient();
        int auctionCounter;

    private:
        int socketTCP;
        struct addrinfo hints, *res;
        int verbose;
        
        
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
        
        void handleOpen(std::string& additionalInfo, int client_socket);
        void handleClose(std::string& additionalInfo, int client_socket);
        void handleBid(std::string& additionalInfo, int client_socket);
        void handleShowAsset(std::string& additionalInfo, int client_socket);

        int parseOpenRequestInfo(std::string& additionalInfo, OpenRequestInfo& openRequestInfo);
        int validateOpenRequestInfo(OpenRequestInfo& openRequestInfo);

        int sendResponse(const std::string& response, int client_socket);

        bool readFData(int& fd, OpenRequestInfo& openRequestInfo);
        bool readTCPdata(int& fd, std::string& request);
};

#endif