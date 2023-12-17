#include "clientTCP.hpp"

ClientTCP::ClientTCP(const char* port, const char* asip_chosen) {
    struct addrinfo hints;
    int errcode;
    this->asip = asip_chosen; 
    struct sigaction act;
    act.sa_handler=SIG_IGN;
  
    if (sigaction (SIGPIPE,&act, NULL)== -1) /*error*/ exit(1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(asip, port, &hints, &res);
    if (errcode != 0) exit(1);

}

ClientTCP::~ClientTCP() {
    freeaddrinfo(res);
}


// Handle Commands


void ClientTCP::handleOpen(std::string& additionalInfo, std::string& uid, std::string& password) {
    //additionalInfo = name asset_fname start_value timeactive

    // check if user is logged in
    if (uid.empty()) {
        std::cout << "login is required\n";
        return;
    }

    AuctionInfo auctionInfo;
    
    // parse additionalInfo
    int isValid = parseOpenInfo(additionalInfo, auctionInfo);
    if (!isValid) return;

    // read file
    auctionInfo.fdata = readFileBinary("src/client/ASSETS/" + auctionInfo.asset_fname);
    if (auctionInfo.fdata.empty()) return;

    // validate file size
    std::string fsizeStr = std::to_string(auctionInfo.fdata.size());
    if (!isFsizeValid(fsizeStr)) return;

    // send request
    if (!sendOpenRequest(uid, password, auctionInfo)){
        if (!closeTCPConn(fd))
            perror("Error closing socket");
        return;
    }

    // receive response
    receiveOpenResponse();
}


void ClientTCP::handleClose(const std::string& additionalInfo, const std::string& uid, const std::string& password) {
    std::string aid = additionalInfo;

    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in" << std::endl;
        return;
    }

    if (!isAidValid(aid)) {
        std::cout << "Invalid AID format" << std::endl;
        return;
    }

    // send request
    if (!sendCloseRequest(uid, password, aid)){
        if (!closeTCPConn(fd))
            perror("Error closing socket");
        return;
    }

    // receive response
    receiveCloseResponse(uid, aid);
}


void ClientTCP::handleShowAsset(const std::string& additionalInfo) {
    std::string aid = additionalInfo;

    if (!isAidValid(aid)) {
        std::cout << "Invalid AID format" << std::endl;
        return;
    }

    if(!sendShowAssetRequest(aid)){
        if (!closeTCPConn(fd))
            perror("Error closing socket");
        return;
    }
    
    receiveShowAssetResponse();
}


void ClientTCP::handleBid(const std::string& additionalInfo, const std::string& uid, const std::string& password) {
    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in" << std::endl;
        return;
    }

    std::string aid = std::string(additionalInfo).substr(0, 3);
    std::string value = std::string(additionalInfo).substr(4);

    if (!isAidValid(aid)) {
        std::cout << "Invalid AID format" << std::endl;
        return;
    }

    if(!sendBidRequest(uid, password, aid, value)){
        if (!closeTCPConn(fd))
            perror("Error closing socket");
        return;  
    }
    receiveBidResponse();
}


// Send Requests


int ClientTCP::sendOpenRequest(std::string& uid, std::string& password, AuctionInfo& auctionInfo) {
    std::ostringstream cmd;
    cmd << "OPA " << uid << " " << password << " " << auctionInfo.name << " "
        << auctionInfo.start_value << " " << auctionInfo.timeactive << " " 
        << auctionInfo.asset_fname << " " << auctionInfo.fdata.size() << " " 
        << auctionInfo.fdata << "\n";

    std::string cmdStr = cmd.str(); 
    ssize_t n, bytes_written = 0;

    if (!createTCPConn(fd, res)){
        perror("Error creating socket");
        return 0;
    }
    
    while (bytes_written < cmdStr.length()) {
        n = write(fd, cmdStr.c_str() + bytes_written, cmdStr.length() - bytes_written); 

        if (n == -1) {
            // Handle error
            if (!closeTCPConn(fd))
                perror("Error closing socket");
            return 0;
        }
        bytes_written += n;
    }

    return 1;
}


int ClientTCP::sendCloseRequest(const std::string& uid, const std::string& password, const std::string& aid) {
    if (!createTCPConn(fd, res)){
        perror("Error creating socket");
        return 0;
    }

    // send request
    
    ssize_t n = write(fd, ("CLS " + uid + " " + password + " " + aid + "\n").c_str(), 24); 
    if(n==-1) return 0;

    return 1;
}


int ClientTCP::sendShowAssetRequest(const std::string& aid) {
    if (!createTCPConn(fd, res)){
        perror("Error creating socket");
        return 0;
    }

    // send request

    ssize_t n = write(fd, ("SAS " + aid + "\n").c_str(), 8); 
    if(n==-1) return 0;

    return 1;
}


int ClientTCP::sendBidRequest(const std::string& uid, const std::string& password, const std::string aid , const std::string value) {
    if (!createTCPConn(fd, res)){
        perror("Error creating socket");
        return 0;
    }

    // send request
    ssize_t n = write(fd, ("BID " + uid + " " + password + " " + aid + " " + value + "\n").c_str(), 25+value.size()); 
    if(n==-1) return 0;
    
    return 1;

}


// Receive Responses


void ClientTCP::receiveOpenResponse() {
    std::string response, response_code, status;

    if (!readTCPdata(fd, response)) {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }
    if (!closeTCPConn(fd)){
        perror("Error closing socket");
        return;
    }

    response_code = std::string(response).substr(0, 3);

    if (response_code != "ROA") {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }
    auto posSpace = response.find(' ', 4);

    status = std::string(response).substr(4, posSpace != std::string::npos ? 
        posSpace - 4 : response.length());


    if (status == "OK"){
        std::string newAID = std::string(response).substr(posSpace + 1); // newAID contains '\n'
        if (newAID.length() != 4 || 
            !std::all_of(newAID.begin(), newAID.end() - 1, ::isdigit)) { // '\n' is ignored
            std::cout << "WARNING: unexpected protocol message\n";
            return;
        }

        std::cout << "Auction opened successfully with Auction ID: " << 
        std::string(response).substr(7, 3) << std::endl;
    }
    else if (status == "NOK\n")
        std::cout << "Auction could not be started" << std::endl;
    else if (status == "NLG\n")
        std::cout << "User not logged in" << std::endl;
    else 
        std::cout << "WARNING: unexpected protocol message\n";

}


void ClientTCP::receiveShowAssetResponse() {
    std::string response;

    // receive response
    if (!readTCPdata(fd, response)) {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    // parse response
    std::string response_code, status, fname, fsize, fdata;

    size_t splitIndex = response.find(' ');
    response_code = response.substr(0, splitIndex);

    if (response_code != "RSA") {
        std::cout << "WARNING: unexpected protocol message\n";
        std::cout << "response_code: " << response_code << std::endl;
        return;
    }
    response = response.substr(splitIndex + 1);
    splitIndex = response.find(' ');
    status = response.substr(0, splitIndex);

    if (status == "NOK\n"){
        std::cout << "problem sending the requested file\n";
        return;
    }
    if (status != "OK"){
        std::cout << "WARNING: unexpected protocol message\n";
        std::cout << "status: " << status << std::endl;
        return;
    }

    response = response.substr(splitIndex + 1);
    splitIndex = response.find(' ');
    fname = response.substr(0, splitIndex);

    if (!isFnameValid(fname)) {
        std::cout << "WARNING: unexpected protocol message\n";
        std::cout << "fname: " << fname << std::endl;
        return;
    }

    response = response.substr(splitIndex + 1);
    splitIndex = response.find(' ');
    fsize = response.substr(0, splitIndex);

    if (!isFsizeValid(fsize)) {
        std::cout << "WARNING: unexpected protocol message\n";
        std::cout << "fsize: " << fsize << std::endl;
        return;
    }

    response = response.substr(splitIndex + 1);
    fdata = response;

    if (!readFData(fd, fsize, fdata)) {
        std::cout << "WARNING: unexpected protocol message\n";
        std::cout << "error reading fdata" << std::endl;
        return;
    }

    if (!closeTCPConn(fd)){
        perror("Error closing socket");
        return;
    }

    std::cout << "File name: " << fname << std::endl;
    std::cout << "File size: " << fsize << std::endl;

    writeFileBinary("src/client/ASSETS/" + fname, fdata);
}


void ClientTCP::receiveBidResponse() {
    ssize_t n;
    std::string response;

    // receive response

    if (!readTCPdata(fd, response)) {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }
    if (!closeTCPConn(fd)){
        perror("Error closing socket");
        return;
    }

    
    // parse received data
    
    if(response.back() != '\n') {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }
    response.pop_back(); // remove final newline

    std::string response_code = std::string(response).substr(0, 3);
    std::string status = std::string(response).substr(4);

    if (response_code != "RBD"){
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    if (status == "ACC") {
        std::cout << "bid placed successfully\n";
    } else if (status == "NOK") {
        std::cout << "auction is not active\n";
    } else if (status == "NLG") {
        std::cout << "user not logged in\n";
    } else if (status == "REF") {
        std::cout << "larger bid has already been placed\n";
    } else if (status == "ILG") {
        std::cout << "cannot place bid in your own auction\n";
    } else {
        std::cout << "WARNING: unexpected protocol message\n" << std::endl;
    }
}

void ClientTCP::receiveCloseResponse(const std::string& uid, const std::string& aid) {
    std::string response, response_code, status;

    if (!readTCPdata(fd, response)) {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }
    if (!closeTCPConn(fd)){
        perror("Error closing socket");
        return;
    }

    response_code = std::string(response).substr(0, 3);
    status = std::string(response).substr(4);

    if (response_code != "RCL"){
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    if (status == "OK\n") 
        std::cout << "auction " << aid << " closed successfully\n";
    else if (status == "NOK\n") 
        std::cout << "user " << uid << " does not exist or the password is incorrect\n";
    else if (status == "NLG\n") 
        std::cout << "user not logged in\n";
    else if (status == "EAU\n") 
        std::cout << "auction " << aid << " does not exist\n";
    else if (status == "EOW\n") 
        std::cout << "auction is not owned by user " << uid << "\n";
    else if (status == "END\n") 
        std::cout << "auction " << aid << " owned by user " << uid << " has already finished\n";
    else 
        std::cout << "WARNING: unexpected protocol message\n";
}


// Validation


int ClientTCP::parseOpenInfo(std::string& additionalInfo, AuctionInfo& auctionInfo) {
    auctionInfo.name = additionalInfo.substr(0, additionalInfo.find(' '));
    additionalInfo = additionalInfo.substr(additionalInfo.find(' ') + 1);

    if (!isAuctionNameValid(auctionInfo.name)) return 0;

    auctionInfo.asset_fname = additionalInfo.substr(0, additionalInfo.find(' '));
    additionalInfo = additionalInfo.substr(additionalInfo.find(' ') + 1);

    if (!isFnameValid(auctionInfo.asset_fname)) return 0;

    auctionInfo.start_value = additionalInfo.substr(0, additionalInfo.find(' '));
    additionalInfo = additionalInfo.substr(additionalInfo.find(' ') + 1);

    if (!isValueValid(auctionInfo.start_value)) return 0;

    auctionInfo.timeactive = additionalInfo;

    if (!isTimeActiveValid(auctionInfo.timeactive)) return 0;

    return 1;
}


int ClientTCP::readTCPdata(int& fd, std::string& response) {
    char buffer[SRC_MESSAGE_SIZE];
    ssize_t n;

    n = read(fd, buffer, SRC_MESSAGE_SIZE);
    response.append(buffer, n);
        
    if (n == -1){
        perror("Error reading from socket");
        return 0; // error whilst reading
    }

    // check last character of response
    if (response.empty())
        return 0;
    
    return 1;
}



int ClientTCP::readFData(int& fd, std::string& fsize, std::string& fdata) {
    char buffer[4096];
    ssize_t n;
    int bytes_to_read = std::stoi(fsize) - fdata.length();

    while (bytes_to_read > 0) {
        n = read(fd, buffer, sizeof(buffer));
        fdata.append(buffer, n);
        bytes_to_read -= n;
    }

    if (n == -1){
        perror("Error reading from socket");
        return 0; // error whilst reading
    }

    // check last character of response
    if (fdata.back() != '\n')
        return 0;

    fdata.pop_back();

    return 1;
}
