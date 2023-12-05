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
    if (fd == -1) exit(1); //TODO remove all exits and handle errors
}

ClientUDP::~ClientUDP() {
    freeaddrinfo(res);
    close(fd);
}

void ClientUDP::handleLogin(const std::string& additionalInfo, std::string& uid, std::string& password) {
    if (!uid.empty()) {  //check if already logged in
        std::cout << "already logged in\n";
        return;
    }

    size_t splitIndex = additionalInfo.find(' ');
    // read value from uid address
    uid = additionalInfo.substr(0, splitIndex);
    password = additionalInfo.substr(splitIndex + 1);

    if (loginValid(uid, password)) {  //check if credentials are valid
        sendLoginRequest(uid, password);
        receiveLoginResponse(uid, password);
    } else {
        std::cout << "invalid login format\n";
        uid = "";
        password = "";
    }
}


void ClientUDP::handleLogout(const std::string& additionalInfo, std::string& uid, std::string& password) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid logout format\n";
        return;
    }
    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }
    sendLogoutRequest(uid, password);
    receiveLogoutResponse(uid, password);
}


void ClientUDP::handleUnregister(const std::string& additionalInfo, std::string& uid, std::string& password) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid unregister format\n";
        return;
    }
    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }
    sendUnregisterRequest(uid, password);
    receiveUnregisterResponse(uid, password);
}


void ClientUDP::handleMyAuctions(const std::string&additionalInfo, std::string& uid) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid command format\n";
        return;
    }

    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }

    sendMyAuctionsRequest(uid);
    receiveMyAuctionsResponse();
}


void ClientUDP::handleMyBids(const std::string& additionalInfo, std::string& uid) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid unregister format\n";
        return;
    }
    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }
    sendMyBidsRequest(uid);
    receiveMyBidsResponse();
}

void ClientUDP::handleAllAuctions(const std::string& additionalInfo, std::string& uid) {
    if (!additionalInfo.empty()) {  //check valid format
        std::cout << "invalid unregister format\n";
        return;
    }
    sendAllAuctionsRequest();
    receiveAllAuctionsResponse();
}

void ClientUDP::handleShowRecord(const std::string& additionalInfo, std::string& uid){
    /*if () {  //check valid format
        std::cout << "invalid  aid\n";
        return;
    }*/ //FIXME validade aid

    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }

    sendShowRecordRequest(additionalInfo);
    receiveShowRecordResponse();
}

//Send Requests


// void ClientUDP::sendAuthRequest(std::string requestType, std::string uid, std::string password) {
//     // send login or logout message to server
//     ssize_t n;
//     char message[21];
//     std::ostringstream msgStream;

//     msgStream << requestType << " " << uid << " " << password << "\n";
//     std::string temp = msgStream.str();

//     strncpy(message, temp.c_str(), sizeof(message) - 1);
//     message[20] = '\0';

//     n = sendto(fd, message, 20, 0, res->ai_addr, res->ai_addrlen);
//     if (n == -1) exit(1);
// }


void ClientUDP::sendLoginRequest(std::string& uid, std::string& password) {
    if (sendto(fd, ("LIN " + uid + " " + password + "\n").c_str(), 6 + uid.length() + password.length(),
        0, res->ai_addr, res->ai_addrlen) == -1) exit(1);
}

void ClientUDP::sendLogoutRequest(std::string& uid, std::string& password) {
    if (sendto(fd, ("LOU " + uid + " " + password + "\n").c_str(), 6 + uid.length() + password.length(),
        0, res->ai_addr, res->ai_addrlen) == -1) exit(1);
}

void ClientUDP::sendUnregisterRequest(std::string& uid, std::string& password) {
    if (sendto(fd, ("UNR " + uid + " " + password + "\n").c_str(), 6 + uid.length() + password.length(),
        0, res->ai_addr, res->ai_addrlen) == -1) exit(1);
}

void ClientUDP::sendMyAuctionsRequest(std::string& uid) {
    if (sendto(fd, ("LMA " + uid + "\n").c_str(), 5 + uid.length(), 0, res->ai_addr, res->ai_addrlen) == -1) exit(1);
}

void ClientUDP::sendMyBidsRequest(std::string& uid) {
    if (sendto(fd, ("LMB " + uid + "\n").c_str(), 5 + uid.length(), 0, res->ai_addr, res->ai_addrlen) == -1) exit(1);
}

void ClientUDP::sendAllAuctionsRequest() {
    if (sendto(fd, "LST\n", 4, 0, res->ai_addr, res->ai_addrlen) == -1) exit(1);
}

void ClientUDP::sendShowRecordRequest(const std::string& aid){
  if (sendto(fd, ("SRC " + aid + "\n").c_str(), 5 + aid.length(), 0, res->ai_addr, res->ai_addrlen) == -1) exit(1);
}


//Receive Responses


void ClientUDP::receiveLoginResponse(std::string& uid, std::string& password){ //FIXME this and logout don't need password
    receiveAuthResponse("RLI", uid, password);
}

void ClientUDP::receiveLogoutResponse(std::string& uid, std::string& password){
    receiveAuthResponse("RLO", uid, password);
}

void ClientUDP::receiveUnregisterResponse(std::string& uid, std::string& password){
    receiveAuthResponse("RUR", uid, password);
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

void ClientUDP::receiveShowRecordResponse(){
    // max size of message is
    //7 + 1 + (6 + 10 + 24 + 6 + 19 + 5 + 5 (spaces))
    // + (1 + 6 + 6 + 19 + 5 + 4(spaces)) * 50
    // + 1 + 19 + 5 + 3(spaces)
    // = 2161 FIXME check this
    char buffer[2161];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd, buffer, 2161, 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) exit(1); // TODO remove all exits and handle errors

    buffer[n] = '\0';

    std::string response_code = std::string(buffer).substr(0, 3);
    std::string response_info = std::string(buffer).substr(4);

    if (response_code != "RRC"){
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
        parseRecordInfo(info);
    }
    else {
        validateShowRecordResponse(response_info);
    }
}


void ClientUDP::receiveAuthResponse(std::string responseType, std::string& uid, std::string& password){
    char buffer[128];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) exit(1);

    buffer[n] = '\0';

    std::string response = std::string(buffer).substr(0, 3);
    std::string status = std::string(buffer).substr(4);

    if (responseType == "RLI"){
        validateLoginResponse(response, status, uid, password);
    }
    else if (responseType == "RLO"){
        validateLogoutResponse(response, status, uid, password);
    }
    else if (responseType == "RUR"){
        validateUnregisterResponse(response, status, uid, password);
    }

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
        std::string status = response_info.substr(0, splitIndex);
        if (status != "OK") {
            std::cout << "WARNING: unexpected protocol message\n";
            return;
        }

        std::string info = response_info.substr(splitIndex + 1);
        parseAuctionInfo(info);
    } 
    else { // no auctions received in the response or error
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
    // info in the format: [ aid state]*
    if (info.empty() || info.back() != '\n') {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    info.pop_back(); // remove newline at the end of the string

    size_t start = 0;
    size_t end = info.find(' ', start + 4);

    while (start != info.length()) {
        std::string segment = (end == std::string::npos) ? info.substr(start) : info.substr(start, end - start);

        if (segment.length() == 5 && 
            isdigit(segment[0]) && isdigit(segment[1]) && isdigit(segment[2]) &&
            segment[3] == ' ' && 
            (segment[4] == '0' || segment[4] == '1')) {

            std::string aid = segment.substr(0, 3);
            char state = segment[4];

            std::cout << "Auction ID: " << aid << ", State: " << (state == '1' ? "Active" : "Inactive") << std::endl;
        } else { // FIXME do the same with parse record info?
            std::cout << "WARNING: unexpected protocol message\n";
        }

        start = (end == std::string::npos) ? info.length() : end + 1;
        end = info.find(' ', start + 4);
    }
}

void ClientUDP::parseRecordInfo(std::string info){
    // info in the format:
    // [host_UID auction_name asset_fname start_value start_date-time timeactive]
    // [ B bidder_UID bid_value bid_date-time bid_sec_time]*
    // [ E end_date-time end_sec_time]
    if (info.empty() || info.back() != '\n') {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    info.pop_back();    // remove newline at the end of the string

    size_t start = 0;
    size_t end;
    //FIXME check if there is no double spaces between fields
    // parse [host_UID auction_name asset_fname start_value start_date-time timeactive]

    std::cout << "RECORD INFO\n" << std::endl;

    for (std::string info_title : {"Host UID", "Auction Name", "Asset", "Start Value", "Start Date", "Time Active"}) {
        if (info_title == "Start Date")
            end = start + 19;
        else
            end = info.find(' ', start);
        
        std::cout << info_title << ": " << info.substr(start, end - start) << std::endl;
        start = end + 1;
    }

    // parse [ B bidder_UID bid_value bid_date-time bid_sec_time]*

    if (start != info.length() && info[start] == 'B') {
        std::cout << std::endl << "BIDS\n";

        int bid_counter = 0;
        while (start != info.length() && info[start] == 'B') {
            start += 2; //skip 'B' character
            
            std::cout << std::endl << "BID #" << ++bid_counter << std::endl;

            for (std::string info_title : {"Bidder UID", "Bid Value", "Bid Date", "Bid Sec Time"}) {
                if (info_title == "Bid Date")
                    end = start + 19;
                else
                    end = info.find(' ', start);
                
                std::cout << info_title << ": " << info.substr(start, end - start) << std::endl;
                start = end + 1;
            }
        }
    }

    // parse [ E end_date-time end_sec_time]

    if (start != info.length() && info[start] == 'E') {
        std::cout << std::endl << "CLOSURE INFO\n" << std::endl;

        start += 2; // skip 'E' character
        std::cout << "End Date: " << info.substr(start, 19) << std::endl;
        std::cout << "End Sec Time: " << info.substr(start+20) << std::endl;
    }
}


// Validations


int ClientUDP::loginValid(std::string& uid, std::string& password) {
    if (uid.length() == 6 && all_of(uid.begin(), uid.end(), ::isdigit)
        && password.length() == 8 && all_of(password.begin(), password.end(), ::isalnum)) {
        return 1;
    } else 
        return 0;
}

void ClientUDP::validateLoginResponse(std::string response, std::string status, std::string& uid, std::string& password){
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

void ClientUDP::validateLogoutResponse(std::string response, std::string status, std::string& uid, std::string& password){
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



void ClientUDP::validateUnregisterResponse(std::string response, std::string status, std::string& uid, std::string& password){
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

void ClientUDP::validateShowRecordResponse(std::string response_info){
    if (response_info == "NOK\n")
            std::cout << "auction does not exist\n";
        else
            std::cout << "WARNING: unexpected protocol message\n";
}