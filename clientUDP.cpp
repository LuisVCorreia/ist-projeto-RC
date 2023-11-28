#include "clientUDP.hpp"

ClientUDP::ClientUDP(const char* port, const char* asip) {
    struct addrinfo hints;
    int errcode;
    this->asip = asip; 

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(asip, port, &hints, &res);
    if (errcode != 0) exit(1);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(1);
}

ClientUDP::~ClientUDP() {
    freeaddrinfo(res);
    close(fd);
}

void ClientUDP::run() {
    while (!std::cin.eof() && !session_terminated) 
        readCommand();
}

void ClientUDP::readCommand() {
    std::string line;
    std::string command;
    std::string additionalInfo;

    std::cout << "> ";
    if (!getline(std::cin, line)) return;

    size_t splitIndex = line.find(' ');

    if (splitIndex != std::string::npos) {
        command = line.substr(0, splitIndex);
        additionalInfo = line.substr(splitIndex + 1); 
    } else 
        command = line;

    if (command == "login") 
        handleLogin(additionalInfo);
    else if (command == "logout")
        handleLogout(additionalInfo);
    else if (command == "unregister")
        handleUnregister(additionalInfo);
    else if (command == "exit") 
        handleExit();
    else 
        std::cout << "command not found\n";
}


void ClientUDP::handleLogin(const std::string& additionalInfo) {
    if (!uid.empty()) {  //check if already logged in
        std::cout << "already logged in\n";
        return;
    }

    size_t splitIndex = additionalInfo.find(' ');
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);

    if (loginValid()) {  //check if credentials are valid
        sendLoginMessage();
        receiveLoginResponse();
    } else {
        std::cout << "invalid login format\n";
        uid = "";
        password = "";
    }
}


void ClientUDP::handleLogout(const std::string& additionalInfo) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid logout format\n";
        return;
    }
    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }
    sendLogoutMessage();
    receiveLogoutResponse();
}


void ClientUDP::handleUnregister(const std::string& additionalInfo) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid unregister format\n";
        return;
    }
    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }
    sendUnregisterMessage();
    receiveUnregisterResponse();

}


void ClientUDP::handleExit() {
    if (!uid.empty()) {  //check if user still logged in
        std::cout << "logout is required\n";
        return;
    }
    session_terminated = true;
}


int ClientUDP::loginValid() {
    if (uid.length() == 6 && all_of(uid.begin(), uid.end(), ::isdigit)
        && password.length() == 8 && all_of(password.begin(), password.end(), ::isalnum)) {
        return 1;
    } else 
        return 0;
}


void ClientUDP::sendAuthMessage(std::string messageType) {
    // send login or logout message to server
    ssize_t n;
    char message[21];
    std::ostringstream msgStream;

    msgStream << messageType << " " << uid << " " << password << "\n";
    std::string temp = msgStream.str();

    strncpy(message, temp.c_str(), sizeof(message) - 1);
    message[20] = '\0';

    n = sendto(fd, message, 20, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) exit(1);
}


void ClientUDP::sendLoginMessage() {
    sendAuthMessage("LIN");
}

void ClientUDP::sendLogoutMessage() {
    sendAuthMessage("LOU");
}

void ClientUDP::sendUnregisterMessage() {
    sendAuthMessage("UNR");
}


void ClientUDP::receiveLoginResponse(){
    char buffer[128];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    
    ssize_t n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) exit(1);

    buffer[n] = '\0';

    std::string response = std::string(buffer).substr(0, 3);
    std::string status = std::string(buffer).substr(4);

    if (response != "RLI"){
        //TODO: se as mensagens não estiverem bem formatadas, ou não corresponderem a 
        // mensagens deste protocolo, então devem ser rejeitadas. Basta mensagem de erro?
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    if (status == "OK\n") 
        std::cout << "successful login\n";
    else if (status == "REG\n") 
        std::cout << "new user registered\n";
    else if (status == "NOK\n"){
        std::cout << "incorrect login attempt\n";
        uid = "";
        password = "";
    }
    else 
        //TODO: Basta mensagem de erro?
        std::cout << "WARNING: unexpected protocol message\n";
}


void ClientUDP::receiveLogoutResponse(){
    char buffer[128];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) exit(1);

    buffer[n] = '\0';

    std::string response = std::string(buffer).substr(0, 3);
    std::string status = std::string(buffer).substr(4);


    if (response != "RLO"){
        //TODO: se as mensagens não estiverem bem formatadas, ou não corresponderem a 
        // mensagens deste protocolo, então devem ser rejeitadas. Basta mensagem de erro?
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    if (status == "OK\n") {
        std::cout << "successful logout\n";
        uid = "";
        password = "";
    }
    else if (status == "NOK\n")
        std::cout << "incorrect logout attempt: user is not logged in\n";
    else if (status == "UNR\n")
        std::cout << "incorrect logout attempt: user is not registered\n";
    else 
        //TODO: Basta mensagem de erro?
        std::cout << "WARNING: unexpected protocol message\n";
    }

void ClientUDP::receiveUnregisterResponse(){
    char buffer[128];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) exit(1);

    buffer[n] = '\0';

    std::string response = std::string(buffer).substr(0, 3);
    std::string status = std::string(buffer).substr(4);


    if (response != "RUR"){
        //TODO: se as mensagens não estiverem bem formatadas, ou não corresponderem a 
        // mensagens deste protocolo, então devem ser rejeitadas. Basta mensagem de erro?
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    if (status == "OK\n") {
        std::cout << "successful unregister\n";
        uid = "";
        password = "";
    }
    else if (status == "NOK\n")
        std::cout << "incorrect unregister attempt: user is not logged in\n";
    else if (status == "UNR\n")
        std::cout << "incorrect unregister attempt: user is not registered\n";
    else 
        //TODO: Basta mensagem de erro?
        std::cout << "WARNING: unexpected protocol message\n";
    }

