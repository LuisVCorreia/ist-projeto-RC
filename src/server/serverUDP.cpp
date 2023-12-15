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
    else
        std::cout << "unknown command\n"; //TODO: fix this

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

        if (!createLogin(uid)) return;
        
        // successfully logged in
        sendResponse("RLI OK\n"); // TODO: check for error // do we need to since it returns anyway?
    }
    else {
        if (!createPassword(uid, password)) return;
        if (!createLogin(uid)) return;

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


// Auxiliary Functions


int ServerUDP::sendResponse(const char* response) {
    if (sendto(socketUDP, response, strlen(response), 0, (const struct sockaddr*)&client_addr, client_addrlen) < 0) {
        std::cout << "WARNING: error sending UDP message\n";
        return 0;
    }

    //closeUDPConn(socketUDP); we dont want to close the socket here

    return 1;
}