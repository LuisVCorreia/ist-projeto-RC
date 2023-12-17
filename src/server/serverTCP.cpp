#include "serverTCP.hpp"

ServerTCP::ServerTCP(const char* port, int& socketTCP) {
    // Create the TCP socket

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
        exit(1); // TODO: Fix Error handling

    socketTCP = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socketTCP == -1)
        exit(1); // TODO: Fix Error handling

    this->socketTCP = socketTCP;

    if (bind(socketTCP, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Bind error TCP server");
        exit(1); // TODO: Fix Error handling
    }

    if (listen(socketTCP, 5) == -1) {  
        perror("Listen error TCP server");
        exit(1); // TODO: Fix Error handling
    }
    freeaddrinfo(res);

    // counter for AID selection
    auctionCounter = getNumAuctions();
}

ServerTCP::~ServerTCP() {
    close(socketTCP);
}


void ServerTCP::handleTCP(){
    while (true) {
        int client_socket = acceptClient();
        if (client_socket >= 0) {
            std::thread client_thread(&ServerTCP::handleClient, 
                                    this, client_socket);
            client_thread.detach();
        }
    }
}


int ServerTCP::acceptClient() {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    // accept the incoming TCP connection
    int client_socket = accept(socketTCP, (struct sockaddr *)&client_addr, &addrlen);
    if (client_socket < 0) {
        perror("Accept error");
        return -1; 
    }

    return client_socket; 
}


void ServerTCP::handleClient(int client_socket) {
    std::string request, command, additionalInfo;

    // set timeout for client socket
    struct timeval tv;
    tv.tv_sec = 5; // TODO: check this?
    tv.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // receive data from the client socket
    readTCPdata(client_socket, request);

    // handle request
    size_t splitIndex = request.find(' ');

    if (splitIndex != std::string::npos) {
        command = request.substr(0, splitIndex);
        additionalInfo = request.substr(splitIndex + 1); 
    } else {
        command = request;
    }

    if (command == "OPA") 
        handleOpen(additionalInfo, client_socket);
    else if (command == "CLS")
        handleClose(additionalInfo, client_socket);
    else if (command == "BID")
        handleBid(additionalInfo, client_socket);
    else if (command == "SAS")
        handleShowAsset(additionalInfo, client_socket);
    else
        sendResponse("ERR\n", client_socket);

    close(client_socket);
}


// Request Handlers


void ServerTCP::handleOpen(std::string& additionalInfo, int client_socket) {
    OpenRequestInfo openRequestInfo;

    // parse and validate info
    if (!parseOpenRequestInfo(additionalInfo, openRequestInfo)) {
        sendResponse("ROA ERR\n", client_socket);
        return;
    }


    if (!readFData(client_socket, openRequestInfo)) {
        sendResponse("ROA ERR\n", client_socket);
        return;
    }

    std::string aid;
    
    commandMutex.lock();
    if (!isUserLogged(openRequestInfo.uid)) {
        sendResponse("ROA NLG\n", client_socket); // user is not logged in
        commandMutex.unlock();
        return;
    }
    
    // get AID for new auction

    if (auctionCounter == 999) { // max number of auctions reached
        sendResponse("ROA NOK\n", client_socket);
        commandMutex.unlock();
        return;
    }
    auctionCounter++;
    aid = std::to_string(auctionCounter);    
    while (aid.length() < 3) aid = "0" + aid; // add leading zeros

    // create auction

    if (!createAuctionDir(aid) || !createNewHost(openRequestInfo.uid, aid)) {
        sendResponse("ROA NOK\n", client_socket);
        auctionCounter--;
        commandMutex.unlock();
        return;
    }

    if (!createStartFile(aid, openRequestInfo.uid, openRequestInfo.name, openRequestInfo.fname, openRequestInfo.start_value, openRequestInfo.timeactive)) {
        sendResponse("ROA NOK\n", client_socket);
        auctionCounter--;
        commandMutex.unlock();
        return;
    }

    if (!createAssetFile(aid, openRequestInfo.fname, openRequestInfo.fdata)) {
        sendResponse("ROA NOK\n", client_socket);
        auctionCounter--;
        commandMutex.unlock();
        return;
    }
    commandMutex.unlock();

    // auction created successfully
    std::ostringstream responseStream;
    responseStream << "ROA OK " << aid << "\n";

    sendResponse(responseStream.str(), client_socket);
}


void ServerTCP::handleClose(std::string& additionalInfo, int client_socket){
    //additionalInfo in the form UID password AID

    if (additionalInfo.back() != '\n') {
        sendResponse("ERR\n", client_socket); // missing final newline
        return;
    }
    
    additionalInfo.pop_back(); // remove final newline

    std::string uid, password, aid;

    size_t splitIndex = additionalInfo.find(' ');

    uid = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    splitIndex = additionalInfo.find(' ');

    password = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    aid = additionalInfo;

    if (!loginValid(uid, password) || !isAidValid(aid)) {
        sendResponse("ERR\n", client_socket);
        return;
    }

    commandMutex.lock();

    // check if user is logged in and if password is correct
    if (!existsUserDir(uid) || !isValidPassword(uid, password)) {
        sendResponse("RCL NOK\n", client_socket);
        commandMutex.unlock();
        return;
    }

    // check if user is logged in
    if (!isUserLogged(uid)) {
        sendResponse("RCL NLG\n", client_socket);
        commandMutex.unlock();
        return;
    }


    // check if auction exists
    if (!existsAuctionDir(aid)) {
        sendResponse("RCL EAU\n", client_socket);
        commandMutex.unlock();
        return;
    }

    // check if auction is owned by user
    if (!checkAuctionOwner(uid, aid)) { 
        sendResponse("RCL EOW\n", client_socket);
        commandMutex.unlock();
        return;
    }

    // check if auction is still active
    if (!isAuctionStillActive(aid)) {
        sendResponse("RCL END\n", client_socket);
        commandMutex.unlock();
        return;
    }


    // close auction
    closeActiveAuction(aid);

    commandMutex.unlock();
    sendResponse("RCL OK\n", client_socket);

}


void ServerTCP::handleShowAsset(std::string& additionalInfo, int client_socket){
    //additionalInfo in the form AID

    if (additionalInfo.back() != '\n') {
        sendResponse("ERR\n", client_socket); // missing final newline
        return;
    }

    additionalInfo.pop_back(); // remove final newline

    std::string aid = additionalInfo;

    if (!isAidValid(aid)) {
        sendResponse("ERR\n", client_socket);
        return;
    }

    std::string fname, fsize, fdata;

    commandMutex.lock();

    if (!getAssetFile(aid, fname, fsize, fdata)) {
        sendResponse("RSA NOK\n", client_socket);
        commandMutex.unlock();
        return;
    }

    commandMutex.unlock();

    // send response
    std::ostringstream responseStream;
    // format is RSA status [Fname Fsize Fdata]
    responseStream << "RSA OK " << fname << " " << fsize << " " << fdata << "\n";

    sendResponse(responseStream.str(), client_socket);

}


void ServerTCP::handleBid(std::string& additionalInfo, int client_socket){
    //additionalInfo in the form UID password AID value

    if (additionalInfo.back() != '\n') {
        sendResponse("ERR\n", client_socket); // missing final newline
        return;
    }
    
    additionalInfo.pop_back(); // remove final newline

    std::string uid, password, aid, value;

    // parse additionalInfo

    size_t splitIndex = additionalInfo.find(' ');

    uid = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    splitIndex = additionalInfo.find(' ');

    password = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    splitIndex = additionalInfo.find(' ');

    aid = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    value = additionalInfo;
   
    // validate fields
    if (!loginValid(uid, password) || !isAidValid(aid) || !isValueValid(value)) {
        sendResponse("ERR\n", client_socket);
        return;
    }
    
    commandMutex.lock();
    // check if user uid and password match
    if (!isValidPassword(uid, password)) {
        sendResponse("ERR\n", client_socket);
        return;
    }
    
    if (!isAuctionStillActive(aid)) {
        sendResponse("RBD NOK\n", client_socket); // auction is not active
        commandMutex.unlock();
        return;
    }

    if (!isUserLogged(uid)) {
        sendResponse("RBD NLG\n", client_socket); // user is not logged in
        commandMutex.unlock();
        return;
    }

    if (getAuctionHost(aid) == uid) {
        sendResponse("RBD ILG\n", client_socket); // user is the host of the auction
        commandMutex.unlock();
        return;
    }
    
    // compare bid value with current value
    int currentValue = getHighestBid(aid);

    if (currentValue == -1) {
        sendResponse("RBD ERR\n", client_socket); //TODO ERR or NOK?
        commandMutex.unlock();
        return;
    }
    
    int valueInt = std::stoi(value);
    if (valueInt <= currentValue) {
        sendResponse("RBD REF\n", client_socket);
        commandMutex.unlock();
        return;
    }
    
    // bid is valid, update auction

    if (!createNewBidder(aid, uid) || !placeBid(aid, uid, value)) {
        sendResponse("RBD ERR\n", client_socket);
        commandMutex.unlock();
        return;
    }
    
    commandMutex.unlock();

    sendResponse("RBD ACC\n", client_socket);
}



// Auxiliary functions


int ServerTCP::sendResponse(const std::string& response, int client_socket) {
    if (write(client_socket, response.c_str(), response.length()) == -1) {
        std::cout << "WARNING: write error\n";
        return 1;
    }
    return 0;
}


int ServerTCP::parseOpenRequestInfo(std::string& additionalInfo, OpenRequestInfo& openRequestInfo) {
    std::string uid, password, name, start_value, timeactive, fname, fsize, fdata;
    size_t splitIndex;

    // uid
    splitIndex = additionalInfo.find(' ');
    if (splitIndex == std::string::npos) return 0;
    uid = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    // password
    splitIndex = additionalInfo.find(' ');
    if (splitIndex == std::string::npos) return 0;
    password = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    // name
    splitIndex = additionalInfo.find(' ');
    if (splitIndex == std::string::npos) return 0;
    name = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    // start_value
    splitIndex = additionalInfo.find(' ');
    if (splitIndex == std::string::npos) return 0;
    start_value = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    // timeactive
    splitIndex = additionalInfo.find(' ');
    if (splitIndex == std::string::npos) return 0;
    timeactive = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    // fname
    splitIndex = additionalInfo.find(' ');
    if (splitIndex == std::string::npos) return 0;
    fname = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    // fsize
    splitIndex = additionalInfo.find(' ');
    if (splitIndex == std::string::npos) return 0;
    fsize = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    // fdata
    fdata = additionalInfo;

    // fill struct
    openRequestInfo.uid = uid;
    openRequestInfo.password = password;
    openRequestInfo.name = name;
    openRequestInfo.start_value = start_value;
    openRequestInfo.timeactive = timeactive;
    openRequestInfo.fname = fname;
    openRequestInfo.fsize = fsize;
    openRequestInfo.fdata = fdata;

    if (!validateOpenRequestInfo(openRequestInfo))
        return 0;
    
    return 1;
}


int ServerTCP::validateOpenRequestInfo(OpenRequestInfo& openRequestInfo) {
    // validate uid and password
    if (!loginValid(openRequestInfo.uid, openRequestInfo.password)) return 0;

    // check if user uid and password match
    if (!isValidPassword(openRequestInfo.uid, openRequestInfo.password)) return 0;

    // validate remaining fields
    if (!isAuctionNameValid(openRequestInfo.name)) return 0;
    if (!isValueValid(openRequestInfo.start_value)) return 0;
    if (!isTimeActiveValid(openRequestInfo.timeactive)) return 0;
    if (!isFnameValid(openRequestInfo.fname)) return 0;
    if (!isFsizeValid((const std::string)openRequestInfo.fsize)) return 0;

    //TODO should we validate if fdata is right size? fsize refers to bytes, not string length

    return 1;
}



bool ServerTCP::readFData(int& fd, OpenRequestInfo& openRequestInfo) {
    char buffer[4096];
    ssize_t n;
    int bytes_to_read = std::stoi(openRequestInfo.fsize) - openRequestInfo.fdata.length();

    while (bytes_to_read > 0) {
        n = read(fd, buffer, sizeof(buffer));
        openRequestInfo.fdata.append(buffer, n);
        bytes_to_read -= n;
    }

    if (n == -1){
        perror("Error reading from socket");
        return false; // error whilst reading
    }

    // check last character of fdata
    if (openRequestInfo.fdata.back()  != '\n')
        return false;

    openRequestInfo.fdata.pop_back();

    return true;
}


bool ServerTCP::readTCPdata(int& fd, std::string& request) {
    char buffer[OPA_MESSAGE_SIZE];
    ssize_t n;

    n = read(fd, buffer, OPA_MESSAGE_SIZE);
    request.append(buffer, n);
        
    if (n == -1){
        perror("Error reading from socket");
        return false; // error whilst reading
    }

    // check last character of request
    if (request.empty())
        return false;
    
    return true;
}