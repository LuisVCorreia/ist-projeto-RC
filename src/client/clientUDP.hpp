#ifndef CLIENTUDP_H
#define CLIENTUDP_H

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

#include <common/common.hpp>

#define HOST "tejo.tecnico.ulisboa.pt"
#define DEFAULT_PORT "58047"
#define DEFAULT_ASIP "127.0.0.1"

class ClientUDP {
    public:
        ClientUDP(const char* port, const char* asip);
        ~ClientUDP();

        void handleLogin(const std::string& additionalInfo, std::string& uid, std::string& password);
        void handleLogout(const std::string& additionalInfo, std::string& uid, std::string& password);
        void handleUnregister(const std::string& additionalInfo, std::string& uid, std::string& password);
        void handleMyAuctions(const std::string&additionalInfo, std::string& uid);
        void handleMyBids(const std::string&additionalInfo, std::string& uid);
        void handleAllAuctions(const std::string&additionalInfo);
        void handleShowRecord(const std::string&additionalInfo, std::string& uid);

        int sendLogoutRequest(std::string& uid, std::string& password);
        void receiveLogoutResponse(std::string& uid, std::string& password);

    private:
        int fd;
        struct addrinfo *res;
        const char* asip;
        
        int sendLoginRequest(std::string& uid, std::string& password);
        int sendUnregisterRequest(std::string& uid, std::string& password);
        int sendMyAuctionsRequest(std::string& uid);
        int sendMyBidsRequest(std::string& uid);
        int sendAllAuctionsRequest();
        int sendShowRecordRequest(const std::string& aid);

        void receiveAuthResponse(std::string requestType, std::string& uid, std::string& password);
        void receiveLoginResponse(std::string& uid, std::string& password);
        void receiveUnregisterResponse(std::string& uid, std::string& password);
        void receiveMyAuctionsResponse();
        void receiveMyBidsResponse();
        void receiveAllAuctionsResponse();
        void receiveListResponse(std::string responseType);
        void receiveShowRecordResponse();

        void validateLogoutResponse(std::string response, std::string status, std::string& uid, std::string& password);
        void validateLoginResponse(std::string response, std::string status, std::string& uid, std::string& password);
        void validateUnregisterResponse(std::string response, std::string status, std::string& uid, std::string& password);
        void validateMyAuctionsResponse(std::string response_info);
        void validateMyBidsResponse(std::string response_info);
        void validateAllAuctionsResponse(std::string response_info);
        void validateShowRecordResponse(std::string response_info);

        void parseAuctionInfo(std::string info);
        void parseRecordInfo(std::string info);
       
};

#endif