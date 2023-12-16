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

    if (listen(socketTCP, 5) == -1) {  // alterar este valor?
        perror("Listen error TCP server");
        exit(1); // TODO: Fix Error handling
    }
    freeaddrinfo(res);

    // counter for AID selection
    auctionCounter = getNumAuctions();
}


void ServerTCP::receiveRequest(int& as_socket){
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    std::string request, command, additionalInfo;

    // Accept the incoming TCP connection
    socketTCP = accept(as_socket, (struct sockaddr *)&client_addr, &addrlen);
    if (socketTCP < 0) {
        perror("Accept error");
        return;
    }

    struct timeval tv;
    tv.tv_sec = 5; // TODO: check this?
    tv.tv_usec = 0;
    setsockopt(socketTCP, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int n;
    char buffer[1024];
    
    while ((n = read(socketTCP, buffer, sizeof(buffer) - 1)) > 0 ) {
        // Null-terminate the received data
        buffer[n] = '\0';

        // Append the received data to the string
        request += buffer;

        if (buffer[n-1] == '\n') break; // message complete
    }

    // send ERR if error during read operation
    if (n==-1){
        sendResponse("ERR\n");
        closeTCPConn(socketTCP);
        return;
    }

    // remove trailing newline character
    request.pop_back();

    // Handle request

    size_t splitIndex = request.find(' ');

    if (splitIndex != std::string::npos) {
        command = request.substr(0, splitIndex);
        additionalInfo = request.substr(splitIndex + 1); 
    } else 
        command = request;


    if (command == "OPA") 
        handleOpen(additionalInfo);
    else if (command == "CLS")
        handleClose(additionalInfo);
    else if (command == "BID")
        handleBid(additionalInfo);
    else
        std::cout << "unknown command\n"; //TODO: fix this

    closeTCPConn(socketTCP);
}


// Request Handlers



void ServerTCP::handleOpen(std::string& additionalInfo) {
    OpenRequestInfo openRequestInfo;

    // parse and validate info
    if (!parseOpenRequestInfo(additionalInfo, openRequestInfo)) {
        sendResponse("ERR\n");
        return;
    }

    if (!isUserLogged(openRequestInfo.uid)) {
        sendResponse("ROA NLG\n"); // user is not logged in
        return;
    }

    // get AID for new auction

    if (auctionCounter == 999) { // max number of auctions reached
        sendResponse("ROA NOK\n");
        return;
    }

    auctionCounter++;
    std::string aid = std::to_string(auctionCounter);
    while (aid.length() < 3) aid = "0" + aid; // add leading zeros

    // create auction

    if (!createAuctionDir(aid) || !createNewHost(openRequestInfo.uid, aid)) {
        sendResponse("ROA NOK\n"); 
        return;
    }

    if (!createStartFile(aid, openRequestInfo.uid, openRequestInfo.name, openRequestInfo.fname, openRequestInfo.start_value, openRequestInfo.timeactive)) {
        sendResponse("ROA NOK\n");
        return;
    }

    if (!createAssetFile(aid, openRequestInfo.fname, openRequestInfo.fdata)) {
        sendResponse("ROA NOK\n");
        return;
    }

    // auction created successfully
    
    char response[12] = "ROA OK ";
    strcat(response, aid.c_str());
    strcat(response, "\n");
    
    sendResponse(response);

    return;
}


void ServerTCP::handleClose(std::string& additionalInfo){
    //additionalInfo in the form UID password AID

    std::string uid, password, aid;

    size_t splitIndex = additionalInfo.find(' ');

    uid = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    splitIndex = additionalInfo.find(' ');

    password = additionalInfo.substr(0, splitIndex);
    additionalInfo = additionalInfo.substr(splitIndex + 1);

    aid = additionalInfo;

    if (!loginValid(uid, password) || !isAidValid(aid)) {
        sendResponse("ERR\n");
        return;
    }

    // check if user is logged in and if password is correct
    if (!existsUserDir(uid) || !isValidPassword(uid, password)) {
        sendResponse("RCL NOK\n");
        return;
    }

    // check if user is logged in
    if (!isUserLogged(uid)) {
        sendResponse("RCL NLG\n");
        return;
    }


    // check if auction exists
    if (!existsAuctionDir(aid)) {
        sendResponse("RCL EAU\n");
        return;
    }

    // check if auction is owned by user
    if (!checkAuctionOwner(uid, aid)) { 
        sendResponse("RCL EOW\n");
        return;
    }

    // check if auction is still active
    if (!isAuctionStillActive(aid)) {
        sendResponse("RCL END\n");
        return;
    }


    // close auction
    closeActiveAuction(aid);
    sendResponse("RCL OK\n");

    return;

}


void ServerTCP::handleBid(std::string& additionalInfo){
    //additionalInfo in the form UID password AID value

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
        sendResponse("ERR\n");
        return;
    }
    
    // check if user uid and password match
    if (!isValidPassword(uid, password)) {
        sendResponse("ERR\n");
        return;
    }

    
    if (!isAuctionStillActive(aid)) {
        sendResponse("RBD NOK\n"); // auction is not active
        return;
    }

    if (!isUserLogged(uid)) {
        sendResponse("RBD NLG\n"); // user is not logged in
        return;
    }

    if (getAuctionHost(aid) == uid) {
        sendResponse("RBD ILG\n"); // user is the host of the auction
        return;
    }
    
    // compare bid value with current value
    int currentValue = getHighestBid(aid);

    if (currentValue == -1) {
        sendResponse("RBD ERR\n"); //TODO ERR or NOK?
        return;
    }
    

    int valueInt = std::stoi(value);
    if (valueInt <= currentValue) {
        sendResponse("RBD REF\n");
        return;
    }
    
    // bid is valid, update auction

    if (!placeBid(aid, uid, value)) {
        sendResponse("RBD ERR\n");
        return;
    }

    sendResponse("RBD OK\n");

    return;
}



// Auxiliary functions


int ServerTCP::sendResponse(const char* response) {
    if (write(socketTCP, response, strlen(response)) == -1) {
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
    if (!isFsizeValid(openRequestInfo.fsize)) return 0;

    //TODO should we validate if fdata is right size? fsize refers to bytes, not string length

    return 1;
}