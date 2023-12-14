#include "Client.hpp"

Client* Client::currentInstance = nullptr; // Initialize the static pointer

Client::Client(const char* port, const char* asip)
    : clientUDP(port, asip), clientTCP(port, asip) {
}

void Client::run() {
    while (!std::cin.eof() && !session_terminated) 
        readCommand();
}

void Client::readCommand() {
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
        clientUDP.handleLogin(additionalInfo, uid, password);
    else if (command == "logout")
        clientUDP.handleLogout(additionalInfo, uid, password);
    else if (command == "unregister")
        clientUDP.handleUnregister(additionalInfo, uid, password);
    else if (command == "myauctions" || command == "ma")
        clientUDP.handleMyAuctions(additionalInfo, uid);
    else if (command == "mybids" || command == "mb")
        clientUDP.handleMyBids(additionalInfo, uid);
    else if (command == "list" || command == "l")
        clientUDP.handleAllAuctions(additionalInfo);
    else if (command == "show_record" || command == "sr")
        clientUDP.handleShowRecord(additionalInfo, uid);
    else if (command == "open")
        clientTCP.handleOpen(additionalInfo, uid, password);
    else if (command == "show_asset" || command == "sa")
        clientTCP.handleShowAsset(additionalInfo);
    else if (command == "bid" || command == "b")
        clientTCP.handleBid(additionalInfo, uid, password);
    else if (command == "close")
        clientTCP.handleClose(additionalInfo, uid, password);
    else if (command == "exit") 
        handleExit();
    else 
        std::cout << "command not found\n";
}

void Client::handleExit() {
    if (!uid.empty()) {  //check if user still logged in
        std::cout << "logout is required\n";
        return;
    }
    session_terminated = true;
}

void Client::setCurrentClientInstance(Client* instance) {
    currentInstance = instance;
}

void Client::sigintHandler(int signum) {
    if (currentInstance != nullptr) {
        currentInstance->logoutIfLoggedIn();
    }
    exit(signum);
}

void Client::logoutIfLoggedIn() {
    std::cout << std::endl;
    // if (!uid.empty()) {
    //     clientUDP.sendLogoutRequest(uid, password);
    //     clientUDP.receiveLogoutResponse(uid, password);
    // }
}





