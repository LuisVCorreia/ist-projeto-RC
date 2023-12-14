#include "common.hpp"

int loginValid(std::string& uid, std::string& password) {
    if (uid.length() == 6 && all_of(uid.begin(), uid.end(), ::isdigit)
        && password.length() == 8 && all_of(password.begin(), password.end(), ::isalnum)) {
        return 1;
    } else 
        return 0;
}


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


