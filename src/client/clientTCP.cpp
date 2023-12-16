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
    if (!sendOpenRequest(uid, password, auctionInfo)) return;

    // receive response
    receiveOpenResponse();
}


void ClientTCP::handleShowAsset(const std::string& additionalInfo) {
    std::string aid = additionalInfo;

    if (!isAidValid(aid)) {
        std::cout << "Invalid AID format" << std::endl;
        return;
    }


    sendShowAssetRequest(additionalInfo);
    receiveShowAssetResponse();
}


void ClientTCP::handleBid(const std::string& additionalInfo, const std::string& uid, const std::string& password) {
    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }

    std::string aid = std::string(additionalInfo).substr(0, 3);
    std::string value = std::string(additionalInfo).substr(4);

    if (!isAidValid(aid)) {
        std::cout << "Invalid AID format" << std::endl;
        return;
    }

    sendBidRequest(uid, password, aid, value);
    receiveBidResponse();
}


void ClientTCP::handleClose(const std::string& additionalInfo, const std::string& uid, const std::string& password) {
    std::string aid = additionalInfo;

    if (uid.empty()) {  //check if user is logged in
        std::cout << "user not logged in\n";
        return;
    }

    if (!isAidValid(aid)) {
        std::cout << "Invalid AID format" << std::endl;
        return;
    }

    // send request
    if (!sendCloseRequest(uid, password, aid)) return;

    // receive response
    receiveCloseResponse(uid, aid);
}


// Send Requests


bool ClientTCP::sendOpenRequest(std::string& uid, std::string& password, AuctionInfo& auctionInfo) {
    std::ostringstream cmd;
    cmd << "OPA " << uid << " " << password << " " << auctionInfo.name << " "
        << auctionInfo.start_value << " " << auctionInfo.timeactive << " " 
        << auctionInfo.asset_fname << " " << auctionInfo.fdata.size() << " " 
        << auctionInfo.fdata << "\n";

    std::string cmdStr = cmd.str(); 

    createTCPConn(fd, res);

    // TODO: Loop needed for write?
    ssize_t n = write(fd, cmdStr.c_str(), cmdStr.length()); // Write to the socket
    if (n == -1) {
        // Handle error
        closeTCPConn(fd);
        return false;
    }
    return true;
}


bool ClientTCP::sendCloseRequest(const std::string& uid, const std::string& password, const std::string& aid) {
    createTCPConn(fd, res);

    // send request
    
    ssize_t n = write(fd, ("CLS " + uid + " " + password + " " + aid + "\n").c_str(), 24); 
    if(n==-1) return false;
    return true;
}


void ClientTCP::sendShowAssetRequest(const std::string& aid) {
    createTCPConn(fd, res);

    // send request

    ssize_t n = write(fd, ("SAS " + aid + "\n").c_str(), 8); 
    if(n==-1)/*error*/exit(1);
}


void ClientTCP::sendBidRequest(const std::string& uid, const std::string& password, const std::string aid , const std::string value) {
    createTCPConn(fd, res);

    // send request
    ssize_t n = write(fd, ("BID " + uid + " " + password + " " + aid + " " + value + "\n").c_str(), 25+value.size()); 
    if(n==-1)/*error*/exit(1);

}


// Receive Responses


void ClientTCP::receiveOpenResponse() {
    std::string response, response_code, status;

    if (!readTCPdata(response)) {
        std::cout << "WARNING: unexpected protocol message\n";
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
    if (!readTCPdata(response)) {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    std::istringstream iss(response);
    std::string response_code, status, fname, fsizeStr;
    iss >> response_code >> status >> fname >> fsizeStr;

    size_t metadata_end = iss.tellg();
    metadata_end = response.find_first_not_of(" ", metadata_end); 

    if (metadata_end == std::string::npos || metadata_end >= response.size()) {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    std::cout << "File name: " << fname << std::endl;
    std::cout << "File size: " << fsizeStr << std::endl;

    // write file data
    size_t fsize = std::stoul(fsizeStr);
    std::string fdata = response.substr(metadata_end, fsize);

    writeFileBinary("src/client/ASSETS/" + fname, fdata);
}


void ClientTCP::receiveBidResponse() {
    ssize_t n;
    std::string response;

    // receive response

    if (!readTCPdata(response)) {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }
    
    // parse received data
    //TODO validate response
    std::string response_code = std::string(response).substr(0, 3);
    std::string status = std::string(response).substr(4, 3);

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
    char buffer[9];
    std::string response, response_code, status;

    if (!readTCPdata(response)) {
        std::cout << "WARNING: unexpected protocol message\n";
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

    if (!isAuctionNameValid(auctionInfo.name)) return false;

    auctionInfo.asset_fname = additionalInfo.substr(0, additionalInfo.find(' '));
    additionalInfo = additionalInfo.substr(additionalInfo.find(' ') + 1);

    if (!isFnameValid(auctionInfo.asset_fname)) return false;

    auctionInfo.start_value = additionalInfo.substr(0, additionalInfo.find(' '));
    additionalInfo = additionalInfo.substr(additionalInfo.find(' ') + 1);

    if (!isValueValid(auctionInfo.start_value)) return false;

    auctionInfo.timeactive = additionalInfo;

    if (!isTimeActiveValid(auctionInfo.timeactive)) return false;

    return true;
}


// Auxiliary Functions


bool ClientTCP::readTCPdata(std::string& response) {
    char buffer[1024];
    ssize_t n;

    while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0 ) {        
        // Null-terminate the received data
        buffer[n] = '\0';

        // Append the received data to the string
        response.append(buffer, n);
    }
    
    closeTCPConn(fd);
    
    if (n == -1){
        perror("Error reading from socket");
        return false; // error whilst reading
}

    //check last character of response
    if (response.empty() || response.back() != '\n')
        return false;
    
    return true;
}