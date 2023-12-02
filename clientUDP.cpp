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
    else if (command == "myauctions" || command == "ma")
        handleMyAuctions(additionalInfo);
    else if (command == "mybids" || command == "mb")
        handleMyBids(additionalInfo);
    else if (command == "list" || command == "l")
        handleAllAuctions(additionalInfo);
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
        sendLoginRequest();
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
    sendLogoutRequest();
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
    sendUnregisterRequest();
    receiveUnregisterResponse();
}


void ClientUDP::handleMyAuctions(const std::string&additionalInfo) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid command format\n";
        return;
    }

    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }

    sendMyAuctionsRequest();
    receiveMyAuctionsResponse();
}


void ClientUDP::handleMyBids(const std::string& additionalInfo) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid unregister format\n";
        return;
    }
    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }
    sendMyBidsRequest();
    receiveMyBidsResponse();
}

void ClientUDP::handleAllAuctions(const std::string& additionalInfo) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid unregister format\n";
        return;
    }
    sendAllAuctionsRequest();
    receiveAllAuctionsResponse();
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


void ClientUDP::sendAuthRequest(std::string requestType) {
    // send login or logout message to server
    ssize_t n;
    char message[21];
    std::ostringstream msgStream;

    msgStream << requestType << " " << uid << " " << password << "\n";
    std::string temp = msgStream.str();

    strncpy(message, temp.c_str(), sizeof(message) - 1);
    message[20] = '\0';

    n = sendto(fd, message, 20, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) exit(1);
}


void ClientUDP::sendLoginRequest() {
    sendAuthRequest("LIN");
}

void ClientUDP::sendLogoutRequest() {
    sendAuthRequest("LOU");
}

void ClientUDP::sendUnregisterRequest() {
    sendAuthRequest("UNR");
}


void ClientUDP::sendUIDRequest(std::string requestType) {
    // send list request to server
    ssize_t n;
    char message[12];
    std::ostringstream msgStream;

    msgStream << requestType << " " << uid << "\n";
    std::string temp = msgStream.str();

    strncpy(message, temp.c_str(), sizeof(message) - 1);
    message[11] = '\0';

    n = sendto(fd, message, 11, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) exit(1);
}


void ClientUDP::sendMyAuctionsRequest() {
    sendUIDRequest("LMA");
}

void ClientUDP::sendMyBidsRequest() {
    sendUIDRequest("LMB");
}

void ClientUDP::sendAllAuctionsRequest() {
    if (sendto(fd, "LST\n", 4, 0, res->ai_addr, res->ai_addrlen) == -1) exit(1);
}


void ClientUDP::receiveAuthResponse(std::string responseType){
    char buffer[128];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) exit(1);

    buffer[n] = '\0';

    std::string response = std::string(buffer).substr(0, 3);
    std::string status = std::string(buffer).substr(4);

    if (responseType == "RLI"){
        validateLoginResponse(response, status);
    }
    else if (responseType == "RLO"){
        validateLogoutResponse(response, status);
    }
    else if (responseType == "RUR"){
        validateUnregisterResponse(response, status);
    }

}

void ClientUDP::receiveLoginResponse(){
    receiveAuthResponse("RLI");
}

void ClientUDP::receiveLogoutResponse(){
    receiveAuthResponse("RLO");
}

void ClientUDP::receiveUnregisterResponse(){
    receiveAuthResponse("RUR");
}


void ClientUDP::validateLoginResponse(std::string response, std::string status){
    if (response != "RLI"){
        //TODO: se as mensagens não estiverem bem formatadas, ou não corresponderem a 
        // mensagens deste protocolo, então devem ser rejeitadas. error message sufficient?
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
        //TODO: error message sufficient?
        std::cout << "WARNING: unexpected protocol message\n";
}



void ClientUDP::validateLogoutResponse(std::string response, std::string status){
    if (response != "RLO"){
        //TODO: se as mensagens não estiverem bem formatadas, ou não corresponderem a 
        // mensagens deste protocolo, então devem ser rejeitadas. error message sufficient?
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
        //TODO: error message sufficient?
        std::cout << "WARNING: unexpected protocol message\n";
}



void ClientUDP::validateUnregisterResponse(std::string response, std::string status){
    if (response != "RUR"){
        //TODO: se as mensagens não estiverem bem formatadas, ou não corresponderem a 
        // mensagens deste protocolo, então devem ser rejeitadas.error message sufficient?
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
        //TODO: error message sufficient?
        std::cout << "WARNING: unexpected protocol message\n";
}



void ClientUDP::receiveMyAuctionsResponse(){
    receiveListResponse("RMA");
}

void ClientUDP::receiveMyBidsResponse(){
    receiveListResponse("RMB");
}

void ClientUDP::receiveAllAuctionsResponse(){
    receiveListResponse("RLS");
}



void ClientUDP::receiveListResponse(std::string responseType){
    // max size of message is 7 + 6 * 999 + 1 =  6002
    char buffer[6002];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd, buffer, 6002, 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) exit(1);   // TODO remove all exits and handle errors

    buffer[n] = '\0';

    std::string response_code = std::string(buffer).substr(0, 3);
    std::string response_info = std::string(buffer).substr(4);

    if (responseType != response_code ){
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    ssize_t splitIndex = response_info.find(' ');

    if (splitIndex != std::string::npos) {
        // no auctions received in the response or error
        std::string status = response_info.substr(0, splitIndex);
        if (status != "OK") {
            std::cout << "WARNING: unexpected protocol message\n";
            return;
        }

        std::string info = response_info.substr(splitIndex + 1);
        parseAuctionInfo(info);
    } 
    else {
        if (responseType == "RMA")
            validateMyAuctionsResponse(response_info);
        else if (responseType == "RMB")
            validateMyBidsResponse(response_info);
        else if (responseType == "RLS")
            validateAllAuctionsResponse(response_info);
        else
            std::cout << "WARNING: unexpected protocol message\n";
    }
}


void ClientUDP::parseAuctionInfo(std::string info){
    // info in the form: [ AID state ]*
    if (info.empty() || info.back() != '\n') {
            std::cout << "WARNING: unexpected protocol message\n";
            return;
        }

        info.pop_back();    // remove newline at the end of the string

        size_t start = 0;
        size_t end = info.find(' ', start + 4);

        while (start != info.length()) {
            std::string segment = (end == std::string::npos) ? info.substr(start) : info.substr(start, end - start);

            if (segment.length() == 5 && 
                isdigit(segment[0]) && isdigit(segment[1]) && isdigit(segment[2]) &&
                segment[3] == ' ' && 
                (segment[4] == '0' || segment[4] == '1')) {

                std::string AID = segment.substr(0, 3);
                char state = segment[4];

                std::cout << "Auction ID: " << AID << ", State: " << (state == '1' ? "Active" : "Inactive") << std::endl;
            } else {
                std::cout << "WARNING: unexpected protocol message\n";
            }

            start = (end == std::string::npos) ? info.length() : end + 1;
            end = info.find(' ', start + 4);
        }

}



void ClientUDP::validateMyAuctionsResponse(std::string response_info){
    if (response_info == "NOK\n")
        std::cout << "user has no ongoing auctions\n";
    else if (response_info == "NLG\n")
        std::cout << "user is not logged in\n";
    else
        std::cout << "WARNING: unexpected protocol message\n";
}


void ClientUDP::validateMyBidsResponse(std::string response_info){
    if (response_info == "NOK\n")
        std::cout << "user has no ongoing bids\n";
    else if (response_info == "NLG\n")
        std::cout << "user is not logged in\n";
    else
        std::cout << "WARNING: unexpected protocol message\n";
}


void ClientUDP::validateAllAuctionsResponse(std::string response_info){
    if (response_info == "NOK\n")
        std::cout << "no auction was yet started\n";
    else{
        std::cout << "WARNING: unexpected protocol message\n";
    }
}