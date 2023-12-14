#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>



int loginValid(std::string& uid, std::string& password);
void createTCPConn(int& fd, struct addrinfo *res);
void closeTCPConn(int& fd);
void createUDPConn(int& fd);
void closeUDPConn(int& fd);

#endif