#include "serverUDP.hpp"

ServerUDP::ServerUDP(const char* port, int& socketUDP) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
        exit(1); // TODO: Fix Error handling

    socketUDP = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socketUDP == -1)
        exit(1); // TODO: Fix Error handling

    if (bind(socketUDP, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Bind error UDP server");
        exit(1); // TODO: Fix Error handling
    }
    freeaddrinfo(res);

}

void ServerUDP::receiveRequest(int& socketUDP){
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    std::string request, command, additionalInfo;

    // Receive data from the UDP socket
    ssize_t received_bytes = recvfrom(socketUDP, buffer, 1024, 0, (struct sockaddr *)&client_addr, &addrlen);
    if (received_bytes < 0) {
        perror("Error receiving UDP data");
        return;
    }

    // Remove final character

    if (buffer[received_bytes-1] != '\n') {
        if (write(socketUDP, "ERR\n", 4) == -1) 
                std::cout << "WARNING: write error\n";
        close(socketUDP);
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
        std::cout << "unknown command\n"; //FIX ME

}

void ServerUDP::handleLogin(std::string& additionalInfo){
    std::string uid, password;

    //TODO
    //validate loginInfo

    size_t splitIndex = additionalInfo.find(' ');
    
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);

    std::cout << "uid: " << uid << "\n";
    std::cout << "password: " << password << "\n";
    
}
