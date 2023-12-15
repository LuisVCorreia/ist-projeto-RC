#include "common.hpp"


void createUDPConn(int& fd) {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(1); //TODO remove all exits and handle errors
}

void closeUDPConn(int& fd) {
    close(fd);
}

void createTCPConn(int& fd, struct addrinfo *res) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) exit(1); // TODO remove all exits and handle errors
    
    int errcode = connect(fd, res->ai_addr, res->ai_addrlen);
    if (errcode == -1) exit(1); //TODO remove all exits and handle errors
}


void closeTCPConn(int& fd) {
    close(fd);
}


// Validations

int loginValid(std::string& uid, std::string& password) {
    if (uid.length() == 6 && all_of(uid.begin(), uid.end(), ::isdigit)
        && password.length() == 8 && all_of(password.begin(), password.end(), ::isalnum)) {
        return 1;
    } else 
        return 0;
}

int isAidValid(std::string& aid) {
    return (aid.length() == 3 && std::all_of(aid.begin(), aid.end(), ::isdigit));
}


int isFnameValid(std::string& fname) {
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
            return 0; 
        }
    } else {
        std::cout << "Invalid Fname format" << std::endl;
        return 0;
    }
    return 1;
}


int isAuctionNameValid(std::string& name) {
    if (name.length() > 10 || name.length() == 0 ||
        !std::all_of(name.begin(), name.end(), ::isalnum)) {
        std::cout << "Invalid description name" << std::endl;
        return 0;
    }
    return 1;
}


int isStartValueValid(std::string& value) {
    if (value.length() > 6 || 
        !std::all_of(value.begin(), value.end(), ::isdigit)) {
        std::cout << "Invalid start value" << std::endl;
        return 0;
    }
    return 1;
}


int isTimeActiveValid(std::string& timeactive) {
    // check timeactive is represented with up to 5 digits
    if (timeactive.length() > 5 || 
        !std::all_of(timeactive.begin(), timeactive.end(), ::isdigit)) {
        std::cout << "Invalid duration of auction" << std::endl;
        return 0;
    }
    return 1;
}

int isFsizeValid(std::string& fsize) {
    // check fsize is represented with up to 8 digits
    if (fsize.length() > 8 || 
        !std::all_of(fsize.begin(), fsize.end(), ::isdigit)) {
        std::cout << "Invalid file size" << std::endl;
        return 0;
    }
    return 1;
}