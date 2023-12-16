#include "serverUDP.hpp"

ServerUDP::ServerUDP(const char* port, int& socketUDP) {
    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
        exit(1); // TODO: Fix Error handling

    socketUDP = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socketUDP == -1)
        exit(1); // TODO: Fix Error handling
    this->socketUDP = socketUDP;

    if (bind(socketUDP, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Bind error UDP server");
        exit(1); // TODO: Fix Error handling
    }
    freeaddrinfo(res);

}

void ServerUDP::receiveRequest(){
    char buffer[1024];
    client_addrlen = sizeof(client_addr);
    std::string request, command, additionalInfo;

    // Receive data from the UDP socket
    ssize_t received_bytes = recvfrom(socketUDP, buffer, 1023, 0, (struct sockaddr*)&client_addr, &client_addrlen);
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
    // else if (command == "SRC")
    //     handleShowRecord(additionalInfo);
    else
        sendResponse("ERR\n");

}


// Request Handlers


void ServerUDP::handleLogin(std::string& additionalInfo){
    std::string uid, password;

    size_t splitIndex = additionalInfo.find(' ');
    
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);
    //TODO loginValid sounds like isValidPassword, maybe change names
    if (!loginValid(uid, password)) { // validate uid and password
        sendResponse("RLI ERR\n");
        return;
    }

    if (!existsUserDir(uid)){
        if (!createUserDir(uid)) // create user directory
            return;
    }

    if (isUserRegistered(uid)) {
        if (!isValidPassword(uid, password)) {
            sendResponse("RLI NOK\n"); // password is incorrect
            return;
        }

        if (!createLogin(uid)){  // error message sent otherwise user never receives response
            sendResponse("RLI ERR\n"); 
            return;
        }
        
        // successfully logged in
        sendResponse("RLI OK\n"); // TODO: check for error // do we need to since it returns anyway?
    }
    else {
        if (!createPassword(uid, password)) return;
        if (!createLogin(uid)){
            sendResponse("RLI ERR\n"); 
            return;
        }

        // successfully registered user
        sendResponse("RLI REG\n"); //TODO what if the user existed but had been unregistered?
    }
}


void ServerUDP::handleLogout(std::string& additionalInfo){
    std::string uid, password;

    size_t splitIndex = additionalInfo.find(' ');
    
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);

    if (!loginValid(uid, password)) { // validate uid and password
        sendResponse("RLO ERR\n");
        return;
    }

    if (!isValidPassword(uid, password)) { // received password is incorrect
        sendResponse("RLO NOK\n"); //TODO send NOK or ERR?
        return;
    }

    if (!existsUserDir(uid)) { // user was not registered
        sendResponse("RLO UNR\n");
        return;
    }

    if (!isUserLogged(uid)) {
        sendResponse("RLO NOK\n"); //TODO send NOK or ERR?
        return;
    }
 
    if (!eraseLogin(uid)) return; // logout user
    sendResponse("RLO OK\n"); //TODO check for error

    return;    
}


void ServerUDP::handleUnregister(std::string& additionalInfo){
    std::string uid, password;

    size_t splitIndex = additionalInfo.find(' ');
    
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);

    if (!loginValid(uid, password)) { // validate uid and password
        sendResponse("RUR ERR\n");
        return;
    }

    if (!isValidPassword(uid, password)) { // received password is incorrect
        sendResponse("RUR NOK\n"); //TODO send NOK or ERR?
        return;
    }

    if (!existsUserDir(uid)) { // user was not registered
        sendResponse("RUR UNR\n");
        return;
    }

    if (!isUserLogged(uid)) { // user is not logged in
        sendResponse("RUR NOK\n");
        return;
    }

    if (!eraseLogin(uid)) return; // logout user
    if (!erasePassword(uid)) return; // unregister user
    sendResponse("RUR OK\n"); //TODO check for error

    return;    
}


void ServerUDP::handleAllAuctions(std::string& additionalInfo){
    if (!additionalInfo.empty()) {
        sendResponse("RLS ERR\n");
        return;
    }

    if (getNumAuctions() == 0) {
        sendResponse("RLS NOK\n");
        return;
    }

    std::string auctions = getAllAuctions();

    if (auctions.empty()){
        sendResponse("RLS ERR\n");
        return;
    }

    sendResponse("RLS OK" + auctions + "\n");
}



void ServerUDP::handleMyAuctions(std::string& additionalInfo){
    std::string uid = additionalInfo;

    if (!existsUserDir(uid)) {
        sendResponse("RMA ERR\n");
        return;
    }

    if (!isUserLogged(uid)) {
        sendResponse("RMA NLG\n");
        return;
    }

    std::string auctions = getMyAuctions(uid);

    if (auctions.empty()){
        sendResponse("RMA NOK\n");
        return;
    }
    sendResponse("RMA OK" + auctions + "\n");
    
}



void ServerUDP::handleMyBids(std::string& additionalInfo){
    std::string uid = additionalInfo;
    
    if (!existsUserDir(uid)) {
        sendResponse("RMB ERR\n");
        return;
    }

    if (!isUserLogged(uid)) {
        sendResponse("RMB NLG\n");
        return;
    }

    std::string auctions = getMyBids(uid);

    if (auctions.empty()){
        sendResponse("RMB NOK\n");
        return;
    }

    sendResponse("RMB OK" + auctions + "\n");
    
}


// void ServerUDP::handleShowRecord(std::string& additionalInfo) {

//     // RRC status [host_UID auction_name asset_fname start_value start_date-time timeactive]
//     // [ B bidder_UID bid_value bid_date-time bid_sec_time]*
//     // [ E end_date-time end_sec_time]

//     std::string aid = additionalInfo;

//     AuctionGeneralInfo general = getAuctionGeneralInfo(aid);

//     std::string response = "RMB OK ";

//     response += host_UID + " " + auction_name + " " + asset_fname + " " +\
//                 start_value + " " + start_datetime + " " + time_active;
    
//     return;
// }


// Auxiliary Functions


int ServerUDP::sendResponse(const std::string& response) {
    if (sendto(socketUDP, response.c_str(), response.length(), 0, (const struct sockaddr*)&client_addr, client_addrlen) < 0) {
        std::cout << "WARNING: error sending UDP message\n";
        return 0;
    }

    //closeUDPConn(socketUDP); we dont want to close the socket here

    return 1;
}