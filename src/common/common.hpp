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
#include <fstream>
#include <sstream>


/*
max size of SRC message is
7 + 1 + (6 + 10 + 24 + 6 + 19 + 5 + 5 (spaces))
+ (1 + 6 + 6 + 19 + 5 + 4(spaces)) * 50
+ 1 + 19 + 5 + 3(spaces)
= 2161 FIXME check this 
*/
#define SRC_MESSAGE_SIZE 2161 

#define RLS_MESSAGE_SIZE 6002 // max size of RLS message is 7 + 6 * 999 + 1 =  6002
#define OPA_MESSAGE_SIZE 78 // exluding fdata size

void createTCPConn(int& fd, struct addrinfo *res);
void closeTCPConn(int& fd);
void createUDPConn(int& fd);
void closeUDPConn(int& fd);

int loginValid(std::string& uid, std::string& password);
int isAidValid(std::string& aid);
int isFnameValid(std::string& fname);
int isAuctionNameValid(std::string& aid);
int isValueValid(std::string& value);
int isTimeActiveValid(std::string& timeactive);
int isFsizeValid(const std::string& fsize);

std::string readFileBinary(const std::string& fname);
int writeFileBinary(const std::string& fname, const std::string& data);

#endif