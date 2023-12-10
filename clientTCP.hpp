#ifndef CLIENTTCP_H
#define CLIENTTCP_H

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
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
        void handleOpen(std::string& additionalInfo, std::string& uid, std::string& password);
        void handleShowAsset(const std::string& additionalInfo);
        void handleBid(const std::string& additionalInfo, const std::string& uid, const std::string& password);
        void handleClose(const std::string& additionalInfo, const std::string& uid, const std::string& password);

        struct AuctionInfo {
            std::string name;
            std::string asset_fname;
            std::string start_value;
            std::string timeactive;
            std::string fdata; 
        };
    
    private:
        int fd;
        struct addrinfo *res;
        std::string asip;

        void createTCPConn();
        void closeTCPConn();
        
        bool isFnameValid(std::string& fname);
        std::string readFileBinary(const std::string& fname);
        bool parseOpenInfo(std::string& additionalInfo, AuctionInfo& auctionInfo);
        bool sendOpenRequest(std::string& uid, std::string& password, AuctionInfo& auctionInfo);
        void receiveOpenResponse();
        void sendShowAssetRequest(const std::string& aid);
        void receiveShowAssetResponse();
        void sendBidRequest(const std::string& uid, const std::string& password, const std::string aid , const std::string value);
        void receiveBidResponse();
        bool sendCloseRequest(const std::string& uid, const std::string& password, const std::string& aid);
        void receiveCloseResponse(const std::string& uid, const std::string& aid);
};

#endif