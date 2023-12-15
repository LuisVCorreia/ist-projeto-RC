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


void createTCPConn(int& fd, struct addrinfo *res);
void closeTCPConn(int& fd);
void createUDPConn(int& fd);
void closeUDPConn(int& fd);

int loginValid(std::string& uid, std::string& password);
int isAidValid(std::string& aid);
int isFnameValid(std::string& fname);
int isAuctionNameValid(std::string& aid);
int isStartValueValid(std::string& value);
int isTimeActiveValid(std::string& timeactive);
int isFsizeValid(std::string& fsize);

std::string readBinaryFile(std::string& fname, std::string& fsize, std::string& fcontent);

#endif