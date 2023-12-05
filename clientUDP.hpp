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
        void handleAllAuctions(const std::string&additionalInfo, std::string& uid);
        void handleShowRecord(const std::string&additionalInfo, std::string& uid);

    private:
        int fd;
        struct addrinfo *res;
        std::string asip;
        
        int loginValid(std::string& uid, std::string& password);
        //void sendAuthRequest(std::string responseType, std::string uid, std::string password);  
        void sendLoginRequest(std::string& uid, std::string& password);
        void receiveLoginResponse(std::string& uid, std::string& password);
        void receiveAuthResponse(std::string requestType, std::string& uid, std::string& password);
        void sendLogoutRequest(std::string& uid, std::string& password);
        void receiveLogoutResponse(std::string& uid, std::string& password);
        void sendUnregisterRequest(std::string& uid, std::string& password);
        void receiveUnregisterResponse(std::string& uid, std::string& password);
        void sendMyAuctionsRequest(std::string& uid);
        void receiveMyAuctionsResponse();
        void sendMyBidsRequest(std::string& uid);
        void receiveMyBidsResponse();
        void sendAllAuctionsRequest();
        void receiveAllAuctionsResponse();
        void receiveListResponse(std::string responseType);
        void sendShowRecordRequest(const std::string& aid);
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