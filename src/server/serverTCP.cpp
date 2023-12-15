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
    auctionCounter = 0;
}


void ServerTCP::receiveRequest(){
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    std::string request, command, additionalInfo;

    // Accept the incoming TCP connection
    int new_sock = accept(socketTCP, (struct sockaddr *)&client_addr, &addrlen);
    if (new_sock < 0) {
        perror("Accept error");
        return;
    }

    struct timeval tv;
    tv.tv_sec = 5; // TODO: check this?
    tv.tv_usec = 0;
    setsockopt(new_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int n;
    char buffer[1024];
    
    while ((n = read(new_sock, buffer, sizeof(buffer) - 1)) > 0 ) {
        // Null-terminate the received data
        buffer[n] = '\0';

        // Append the received data to the string
        request += buffer;

        if (buffer[n-1] == '\n') break; // message complete
    }

    // send ERR if error during read operation
    if (n==-1){
        sendResponse("ERR\n", new_sock);
        close(new_sock);
        return;
    }


    // Handle request

    size_t splitIndex = request.find(' ');

    if (splitIndex != std::string::npos) {
        command = request.substr(0, splitIndex);
        additionalInfo = request.substr(splitIndex + 1); 
    } else 
        command = request;


    if (command == "OPA") 
        handleOpen(additionalInfo, new_sock);
    else
        std::cout << "unknown command\n"; //TODO: fix this

    close(new_sock);
}


// Request Handlers

struct OpenRequestInfo { //TODO wasnt working in hpp, why
            std::string uid;
            std::string password;
            std::string name;
            std::string start_value;
            std::string timeactive;
            std::string fname;
            std::string fsize;
            std::string fdata; 
        };

void ServerTCP::handleOpen(std::string& additionalInfo, int new_sock) {
    OpenRequestInfo openRequestInfo;

    // parse and validate info
    if (!parseOpenRequestInfo(additionalInfo, openRequestInfo)) {
        sendResponse("ERR\n", new_sock);
        return;
    }


    if (!isUserLogged(openRequestInfo.uid)) {
        sendResponse("ROA NLG\n", new_sock); // user is not logged in
        return;
    }


    // get AID for new auction

    if (auctionCounter == 999) { // max number of auctions reached
        sendResponse("ROA NOK\n", new_sock);
        return;
    }

    auctionCounter++;
    std::string aid = std::to_string(auctionCounter);
    while (aid.length() < 3) aid = "0" + aid; // add leading zeros

    // create auction

    if (!createAuctionDir(aid)) {
        sendResponse("NOK\n", new_sock);
        return;
    }

    if (!createStartFile(aid, openRequestInfo.uid, openRequestInfo.name, openRequestInfo.fname, openRequestInfo.start_value, openRequestInfo.timeactive)) {
        sendResponse("NOK\n", new_sock);
        return;
    }

    if (!createAssetFile(aid, openRequestInfo.fname, openRequestInfo.fdata)) {
        sendResponse("NOK\n", new_sock);
        return;
    }

    // auction created successfully
    
    char response[12] = "ROA OK ";
    strcat(response, aid.c_str());
    strcat(response, "\n");
    
    sendResponse(response, new_sock);

    return;
}


// Auxiliary functions


int ServerTCP::sendResponse(const char* response, int new_sock) {
    if (write(new_sock, response, strlen(response)) == -1) {
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
    if (!isStartValueValid(openRequestInfo.start_value)) return 0;
    if (!isTimeActiveValid(openRequestInfo.timeactive)) return 0;
    if (!isFnameValid(openRequestInfo.fname)) return 0;
    if (!isFsizeValid(openRequestInfo.fsize)) return 0;

    //TODO should we validate if fdata is right size? fsize refers to bytes, not string length

    return 1;
}