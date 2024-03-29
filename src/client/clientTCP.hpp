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

#include <common/common.hpp>

#define HOST "tejo.tecnico.ulisboa.pt"
#define DEFAULT_PORT "58047"
#define DEFAULT_ASIP "127.0.0.1"

class ClientTCP {
    public:
        ClientTCP(const char* port, const char* asip);
        ~ClientTCP();
        void handleOpen(std::string& additionalInfo, std::string& uid, std::string& password);
        void handleClose(const std::string& additionalInfo, const std::string& uid, const std::string& password);
        void handleShowAsset(const std::string& additionalInfo);
        void handleBid(const std::string& additionalInfo, const std::string& uid, const std::string& password);
    
    private:
        int fd;
        struct addrinfo *res;
        const char* asip;

        struct AuctionInfo {
            std::string name;
            std::string asset_fname;
            std::string start_value;
            std::string timeactive;
            std::string fdata; 
        };

        int sendOpenRequest(std::string& uid, std::string& password, AuctionInfo& auctionInfo);
        int sendCloseRequest(const std::string& uid, const std::string& password, const std::string& aid);
        int sendShowAssetRequest(const std::string& aid);
        int sendBidRequest(const std::string& uid, const std::string& password, const std::string aid , const std::string value);
        
        void receiveOpenResponse();
        void receiveCloseResponse(const std::string& uid, const std::string& aid);
        void receiveShowAssetResponse();
        void receiveBidResponse();

        int parseOpenInfo(std::string& additionalInfo, AuctionInfo& auctionInfo);

        int readTCPdata(int& fd, std::string& response);
        int readFData(int& fd, std::string& fsize, std::string& fdata);
        
};

#endif