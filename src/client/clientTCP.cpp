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


void ClientTCP::createTCPConn() {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) exit(1); // TODO remove all exits and handle errors
    
    int errcode = connect(fd, res->ai_addr, res->ai_addrlen);
    if (errcode == -1) exit(1); //TODO remove all exits and handle errors
}

void ClientTCP::closeTCPConn() {
    close(fd);
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
    bool isValid = parseOpenInfo(additionalInfo, auctionInfo);
    if (!isValid) return;

    // read file
    auctionInfo.fdata = readFileBinary(auctionInfo.asset_fname);
    if (auctionInfo.fdata.empty()) return;

    // validate file size
    if (auctionInfo.fdata.size() > 10 * 1024 * 1024) {
        std::cout << "File size exceeds 10 MB limit." << std::endl;
        return;
    }

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

    createTCPConn();

    // TODO: Loop needed for write?
    ssize_t n = write(fd, cmdStr.c_str(), cmdStr.length()); // Write to the socket
    if (n == -1) {
        // Handle error
        closeTCPConn();
        return false;
    }
    return true;
}


bool ClientTCP::sendCloseRequest(const std::string& uid, const std::string& password, const std::string& aid) {
    createTCPConn();

    // send request
    
    ssize_t n = write(fd, ("CLS " + uid + " " + password + " " + aid + "\n").c_str(), 24); 
    if(n==-1) return false;
    return true;
}


void ClientTCP::sendShowAssetRequest(const std::string& aid) {
    createTCPConn();

    // send request

    ssize_t n = write(fd, ("SAS " + aid + "\n").c_str(), 8); 
    if(n==-1)/*error*/exit(1);
}


void ClientTCP::sendBidRequest(const std::string& uid, const std::string& password, const std::string aid , const std::string value) {
    createTCPConn();

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


void ClientTCP::receiveShowAssetResponse() { //TODO receiving images not working
    ssize_t n;
    std::string response;
    std::string response_code, status, fname, fsizeStr, fdata;

   // receive response

    if (!readTCPdata(response)) {
        std::cout << "WARNING: unexpected protocol message\n";
        return;
    }

    // parse received data
    
    std::istringstream iss(response);
    iss >> response_code >> status >> fname >> fsizeStr;
    
    std::cout << "File name: " << fname << std::endl;
    std::cout << "File size: " << fsizeStr << std::endl;
    //TODO also show file directory (?)

    // convert fsizeStr to integer

    size_t fsize = std::stoul(fsizeStr);

    // write file data
    std::ofstream outfile;
    
    outfile.open(fname, std::ios::binary);

    outfile.write(response.c_str() + iss.tellg() + 1, static_cast<std::streamsize>(fsize));

    outfile.close();
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


bool ClientTCP::isAidValid(std::string& aid) {
    return (aid.length() == 3 && std::all_of(aid.begin(), aid.end(), ::isdigit));
}


bool ClientTCP::isFnameValid(std::string& fname) {
    // check fname is limited to a total of 24 alphanumerical characters 
    // (plus ‘-‘, ‘_’ and ‘.’), including the separating dot and the 3-letter extension: “nnn…nnnn.xxx”.

    auto len = fname.length();
    if (len <= 24 && len >= 5 && fname[len - 4] == '.') {

        auto isValidChar = [](char c) { // lambda function
            return std::isalnum(c) || c == '-' || c == '_' || c == '.';
        };

        if (!(std::all_of(fname.begin(), fname.end() - 4, isValidChar)) || 
            !(std::all_of(fname.end() - 3, fname.end(), ::isalnum))) {
            std::cout << "Invalid Fname format" << std::endl;
            return false; 
        }
    } else {
        std::cout << "Invalid Fname format" << std::endl;
        return false;
    }
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
        response += buffer;
    }
    
    closeTCPConn();
    
    if (n == -1) return false; // error whilst reading

    //check last character of response
    if (response.empty() || response.back() != '\n')
        return false;
    
    return true;
}


std::string ClientTCP::readFileBinary(const std::string& fname) {
    std::ifstream file("src/client/" + fname, std::ios::binary);
    if (!file) {
        std::cout << "Cannot open file: " << fname << std::endl;
        return "";
    }

    std::ostringstream oss;
    oss << file.rdbuf(); // read the file

    if (!oss) {
        std::cout << "Failed to read file: " << fname << std::endl;
        return "";
    }
    return oss.str();
}


bool ClientTCP::parseOpenInfo(std::string& additionalInfo, AuctionInfo& auctionInfo) {
    auctionInfo.name = additionalInfo.substr(0, additionalInfo.find(' '));
    additionalInfo = additionalInfo.substr(additionalInfo.find(' ') + 1);

    // check name is up to 10 alphanumeric characters
    if (auctionInfo.name.length() > 10 || auctionInfo.name.length() == 0 ||
        !std::all_of(auctionInfo.name.begin(), auctionInfo.name.end(), ::isalnum)) {
        std::cout << "Invalid description name" << std::endl;
        return false;
    }

    auctionInfo.asset_fname = additionalInfo.substr(0, additionalInfo.find(' '));
    additionalInfo = additionalInfo.substr(additionalInfo.find(' ') + 1);

    if (!isFnameValid(auctionInfo.asset_fname)) return false;

    auctionInfo.start_value = additionalInfo.substr(0, additionalInfo.find(' '));
    additionalInfo = additionalInfo.substr(additionalInfo.find(' ') + 1);

    // check start_value is represented with up to 6 digits
    if (auctionInfo.start_value.length() > 6 || 
        !std::all_of(auctionInfo.start_value.begin(), auctionInfo.start_value.end(), ::isdigit)) {
        std::cout << "Invalid start value" << std::endl;
        return false;
    }

    auctionInfo.timeactive = additionalInfo;

    // check timeactive is represented with up to 5 digits
    if (auctionInfo.timeactive.length() > 5 || 
        !std::all_of(auctionInfo.timeactive.begin(), auctionInfo.timeactive.end(), ::isdigit)) {
        std::cout << "Invalid duration of auction" << std::endl;
        return false;
    }

    return true;
}