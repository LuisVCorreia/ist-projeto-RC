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
        void run();

    private:
        int fd;
        struct addrinfo *res;
        std::string asip;
        bool session_terminated = false;
        std::string uid;
        std::string password;
        
        void readCommand();
        void handleLogin(const std::string& additionalInfo);
        void handleLogout(const std::string& additionalInfo);
        void handleUnregister(const std::string& additionalInfo);
        void handleMyAuctions(const std::string&additionalInfo);
        void handleMyBids(const std::string&additionalInfo);
        void handleAllAuctions(const std::string&additionalInfo);
        void handleExit();
        int loginValid();
        void sendAuthRequest(std::string responseType);  
        void sendLoginRequest();
        void receiveLoginResponse();
        void receiveAuthResponse(std::string requestType);
        void validateUnregisterResponse(std::string response, std::string status);
        void sendLogoutRequest();
        void receiveLogoutResponse();
        void sendUnregisterRequest();
        void receiveUnregisterResponse();
        void sendMyAuctionsRequest();
        void sendUIDRequest(std::string requestType);
        void receiveMyAuctionsResponse();
        void sendMyBidsRequest();
        void receiveMyBidsResponse();
        void sendAllAuctionsRequest();
        void receiveAllAuctionsResponse();
        void receiveListResponse(std::string responseType);
        void validateLogoutResponse(std::string response, std::string status);
        void validateLoginResponse(std::string response, std::string status);
        void validateMyAuctionsResponse(std::string response, std::string status);

        void validateMyAuctionsResponse(std::string response_info);
        void validateMyBidsResponse(std::string response_info);
        void validateAllAuctionsResponse(std::string response_info);

        void parseAuctionInfo(std::string info);

};

#endif