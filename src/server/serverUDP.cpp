#include "serverUDP.hpp"

ServerUDP::ServerUDP(const char* port, int& socketUDP) {
    struct addrinfo hints;
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
    this->res = res;

    if (bind(socketUDP, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Bind error UDP server");
        exit(1); // TODO: Fix Error handling
    }
    freeaddrinfo(res);

}

void ServerUDP::receiveRequest(int& socketUDP){
    char buffer[1024];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    std::string request, command, additionalInfo;

    // Receive data from the UDP socket
    ssize_t received_bytes = recvfrom(socketUDP, buffer, 1023, 0, (struct sockaddr*)&addr, &addrlen);
    if (received_bytes < 0) {
        perror("Error receiving UDP data");
        return;
    }

    // Remove final character

    if (buffer[received_bytes-1] != '\n') {
        if (sendto(socketUDP, "ERR\n", 4, 0, res->ai_addr, res->ai_addrlen) < 0)
            std::cout << "WARNING: error sending UDP message\n";
        closeUDPConn(socketUDP);
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
    else
        std::cout << "unknown command\n"; //TODO: fix this

}

void ServerUDP::handleLogin(std::string& additionalInfo){
    std::string uid, password;

    size_t splitIndex = additionalInfo.find(' ');
    
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);

    if (!loginValid(uid, password)) { // validate uid and password
        if (sendto(socketUDP, "RLI ERR\n", 8, 0, res->ai_addr, res->ai_addrlen) < 0)
            std::cout << "WARNING: error sending UDP message\n";
        closeUDPConn(socketUDP); // TODO: check if error?
        return;
    }

    // create directory (uidx)
    
    if (!existsUserDir(uid)){
        if (!createUserDir(uid)) // create user directory
            return;
    }

    if (isUserRegistered(uid)) {
        if (!isValidPassword(uid, password)) {
            sendResponse("RLI NOK\n") // password is incorrect
            return;
        }

        if (!createLogin(uid)) return;
        
        // successfully logged in
        if (!sendResponse("RLI OK\n")) return;
    }
    else {
        if (!createPassword(uid, password)) return;
        if (!createLogin(uid)) return;

        // successfully registered user
        if (!sendResponse("RLI REG\n")) return; //TODO what if the user existed but had been unregistered?
    }
}


void ServerUDP::handleLogout(std::string& additionalInfo){
    std::string uid, password;

    size_t splitIndex = additionalInfo.find(' ');
    
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);

    if (!loginValid(uid, password)) { // validate uid and password
        if (sendto(socketUDP, "RLI ERR\n", 8, 0, res->ai_addr, res->ai_addrlen) < 0)
            std::cout << "WARNING: error sending UDP message\n";
        closeUDPConn(socketUDP); // TODO: check if error?
        return;
    }

    //if (!isValidPassword(uid, password))

    
    
}


int sendResponse(const char* response) {
    if (sendto(socketUDP, "RLI NOK\n", 8, 0, res->ai_addr, res->ai_addrlen) < 0) {
        std::cout << "WARNING: error sending UDP message\n";
        return 0;
    }

    if (!closeUDPConn(socketUDP)) {
        std::cout << "WARNING: error closing UDP socket\n";
        return 0;
    }
    return 1;
}