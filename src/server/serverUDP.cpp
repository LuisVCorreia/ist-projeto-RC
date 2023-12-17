#include "serverUDP.hpp"

ServerUDP::ServerUDP(const char* port, int& socketUDP, int verbose) {
    this->verbose = verbose;

    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        perror("getaddrinfo error TCP server");
        exit(1);
    }

    socketUDP = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socketUDP == -1){
        perror("socket error UDP server");
        exit(1);
    }
    this->socketUDP = socketUDP;

    if (bind(socketUDP, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind error UDP server");
        exit(1);
    }
    freeaddrinfo(res);

}

ServerUDP::~ServerUDP() {
    close(socketUDP);
}


void ServerUDP::handleUDP(){
    while (true) 
        receiveRequest(); 
}




void ServerUDP::receiveRequest(){
    char buffer[SRC_MESSAGE_SIZE+1];
    client_addrlen = sizeof(client_addr);
    std::string request, command, additionalInfo;

    // Receive data from the UDP socket
    ssize_t received_bytes = recvfrom(socketUDP, buffer, SRC_MESSAGE_SIZE, 0, (struct sockaddr*)&client_addr, &client_addrlen);
    if (received_bytes < 0) {
        perror("Error receiving UDP data");
        return;
    }

    // Remove final character

    if (buffer[received_bytes-1] != '\n'){
        sendResponse("ERR\n");
        return;
    }

    buffer[received_bytes] = '\0';

    request = buffer;
    request.pop_back(); // remove '\n'

    // Handle request

    size_t splitIndex = request.find(' ');

    if (splitIndex != std::string::npos) {
        command = request.substr(0, splitIndex);
        additionalInfo = request.substr(splitIndex + 1); 
    } else 
        command = request;


    if (command == "LIN") 
        handleLogin(additionalInfo);
    else if (command == "LOU")
        handleLogout(additionalInfo);
    else if (command == "UNR")
        handleUnregister(additionalInfo);
    else if (command == "LST")
        handleAllAuctions(additionalInfo);
    else if (command == "LMA")
        handleMyAuctions(additionalInfo);
    else if (command == "LMB")
        handleMyBids(additionalInfo);
    else if (command == "SRC")
        handleShowRecord(additionalInfo);
    else
        sendResponse("ERR\n");

}


// Request Handlers


void ServerUDP::handleLogin(std::string& additionalInfo){
    std::string uid, password;

    size_t splitIndex = additionalInfo.find(' ');
    
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);
    
    if (!loginValidFormat(uid, password)) { // validate uid and password
        sendResponse("RLI ERR\n");
        return;
    }

    commandMutex.lock();

    if (!existsUserDir(uid)){
        if (!createUserDir(uid)) { // create user directory
            sendResponse("RLI ERR\n");
            commandMutex.unlock();
            return;
        }
    }

    if (isUserRegistered(uid)) {
        if (!isValidPassword(uid, password)) {
            sendResponse("RLI NOK\n"); // password is incorrect
            commandMutex.unlock();
            return;
        }

        if (!createLogin(uid)){  // error message sent otherwise user never receives response
            sendResponse("RLI ERR\n"); 
            commandMutex.unlock();
            return;
        }
        commandMutex.unlock();
        
        // successfully logged in
        sendResponse("RLI OK\n");
    }
    else {
        if (!createPassword(uid, password)){
            sendResponse("RLI ERR\n"); 
            commandMutex.unlock();
            return;
        }
        if (!createLogin(uid)){
            sendResponse("RLI ERR\n"); 
            commandMutex.unlock();
            return;
        }
        commandMutex.unlock();

        // successfully registered user
        sendResponse("RLI REG\n");
    }
}


void ServerUDP::handleLogout(std::string& additionalInfo){
    std::string uid, password;

    size_t splitIndex = additionalInfo.find(' ');
    
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);

    if (!loginValidFormat(uid, password)) { // validate uid and password
        sendResponse("RLO ERR\n");
        return;
    }

    commandMutex.lock();

    if (!isValidPassword(uid, password)) { // received password is incorrect
        sendResponse("RLO ERR\n"); 
        commandMutex.unlock();
        return;
    }

    if (!existsUserDir(uid)) { // user was not registered
        sendResponse("RLO UNR\n");
        commandMutex.unlock();
        return;
    }

    if (!isUserLogged(uid)) {
        sendResponse("RLO NOK\n"); 
        commandMutex.unlock();
        return;
    }
 
    if (!eraseLogin(uid)){
        sendResponse("RLO ERR\n");
        commandMutex.unlock();
        return; 
    }

    commandMutex.unlock();
    sendResponse("RLO OK\n");
    
}


void ServerUDP::handleUnregister(std::string& additionalInfo){
    std::string uid, password;

    size_t splitIndex = additionalInfo.find(' ');
    
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);

    if (!loginValidFormat(uid, password)) { // validate uid and password
        sendResponse("RUR ERR\n");
        return;
    }

    commandMutex.lock();

    if (!isValidPassword(uid, password)) { // received password is incorrect
        sendResponse("RUR NOK\n");
        commandMutex.unlock();
        return;
    }

    if (!existsUserDir(uid)) { // user was not registered
        sendResponse("RUR UNR\n");
        commandMutex.unlock();
        return;
    }

    if (!isUserLogged(uid)) { // user is not logged in
        sendResponse("RUR NOK\n");
        commandMutex.unlock();
        return;
    }
    
    if (!eraseLogin(uid)) {
        sendResponse("RLO ERR\n");
        commandMutex.unlock();
        return;
    }
    if (!erasePassword(uid)) {
        sendResponse("RLO ERR\n");
        commandMutex.unlock();
        return;
    }

    commandMutex.unlock();
    sendResponse("RUR OK\n");
    
}


void ServerUDP::handleAllAuctions(std::string& additionalInfo){
    if (!additionalInfo.empty()) {
        sendResponse("RLS ERR\n");
        return;
    }

    commandMutex.lock();

    if (getNumAuctions() == 0) {
        sendResponse("RLS NOK\n");
        commandMutex.unlock();
        return;
    }

    std::string auctions = getAllAuctions();

    commandMutex.unlock();

    if (auctions.empty()){
        sendResponse("RLS ERR\n");
        return;
    }

    sendResponse("RLS OK" + auctions + "\n");
}



void ServerUDP::handleMyAuctions(std::string& additionalInfo){
    std::string uid = additionalInfo;

    commandMutex.lock();

    if (!existsUserDir(uid)) {
        sendResponse("RMA ERR\n");
        commandMutex.unlock();
        return;
    }

    if (!isUserLogged(uid)) {
        sendResponse("RMA NLG\n");
        commandMutex.unlock();
        return;
    }

    std::string auctions = getMyAuctions(uid);

    commandMutex.unlock();

    if (auctions.empty()){
        sendResponse("RMA NOK\n");
        return;
    }

    sendResponse("RMA OK" + auctions + "\n");
    
}



void ServerUDP::handleMyBids(std::string& additionalInfo){
    std::string uid = additionalInfo;
    
    commandMutex.lock();

    if (!existsUserDir(uid)) {
        sendResponse("RMB ERR\n");
        commandMutex.unlock();
        return;
    }

    if (!isUserLogged(uid)) {
        sendResponse("RMB NLG\n");
        commandMutex.unlock();
        return;
    }

    std::string auctions = getMyBids(uid);

    commandMutex.unlock();

    if (auctions.empty()){
        sendResponse("RMB NOK\n");
        return;
    }

    sendResponse("RMB OK" + auctions + "\n");
    
}


void ServerUDP::handleShowRecord(std::string& additionalInfo) {

    // RRC status [host_UID auction_name asset_fname start_value start_date-time timeactive]
    // [ B bidder_UID bid_value bid_date-time bid_sec_time]*
    // [ E end_date-time end_sec_time]

    std::string aid = additionalInfo;

    if (!isAidValid(aid)) {
        sendResponse("RRC ERR\n");
        return;
    }

    commandMutex.lock();

    if (!existsAuctionDir(aid)) {
        sendResponse("RRC NOK\n");
        commandMutex.unlock();
        return;
    }

    AuctionGeneralInfo general;
    BidList bidList;
    AuctionEndInfo endInfo;

    int active = isAuctionStillActive(aid);

    if (!getAuctionGeneralInfo(aid, general)) {
        sendResponse("RRC NOK\n");
        commandMutex.unlock();
        return;
    };

    getBidList(aid, bidList);
    
    if (!active) {
        if (!getAuctionEndInfo(aid, endInfo)) {
            sendResponse("RRC NOK\n");
            commandMutex.unlock();
            return;
        }
    }   

    commandMutex.unlock();

    // write response

    std::string response = "RRC OK ";

    //general info
    response += general.host_uid + " " + general.auction_name + " " + general.asset_fname + " " +\
                general.start_value + " " + general.start_datetime + " " + general.time_active;

    //bids info
    
    if (bidList.bid_count > 0) {
        BidInfo bid;
        for (int i = 0; i < bidList.bid_count; i++) {
            bid = bidList.bids[i];
            response += " B " + bid.bidder_uid + " " + bid.bid_value + " " + bid.bid_datetime + " " + bid.bid_sec_time;
        }
    }

    //end info
    if (!active)
        response += " E " + endInfo.end_datetime + " " + endInfo.end_sec_time;

    response += "\n";

    sendResponse(response.c_str());

}


// Auxiliary Functions


int ServerUDP::sendResponse(const std::string& response) {
    if (sendto(socketUDP, response.c_str(), response.length(), 0, (const struct sockaddr*)&client_addr, client_addrlen) < 0) {
        std::cout << "WARNING: error sending UDP message\n";
        return 0;
    }

    return 1;
}