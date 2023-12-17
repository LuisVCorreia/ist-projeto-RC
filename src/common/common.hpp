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
3 + 2 + 2(spaces) + (6 + 10 + 24 + 6 + 19 + 5 + 5(spaces))
+ (1 + 6 + 6 + 19 + 5 + 5(spaces)) * 50
+ 1 + 19 + 5 + 3(spaces) + 1(newline)
= 2211
*/
#define SRC_MESSAGE_SIZE 2211
#define AUTH_MESSAGE_SIZE 20 // max size of LIN, LOU or UNR message is 3 + 1 + 6 + 1 + 8 + 1
#define RLS_MESSAGE_SIZE 6002 // max size of RLS message is 7 + 6 * 999 + 1 =  6002
#define OPA_MESSAGE_SIZE 78 // excluding fdata size

int createTCPConn(int& fd, struct addrinfo *res);
int closeTCPConn(int& fd);
int createUDPConn(int& fd);
int closeUDPConn(int& fd);

int loginValidFormat(std::string& uid, std::string& password);
int isAidValid(std::string& aid);
int isFnameValid(std::string& fname);
int isAuctionNameValid(std::string& aid);
int isValueValid(std::string& value);
int isTimeActiveValid(std::string& timeactive);
int isFsizeValid(const std::string& fsize);

std::string readFileBinary(const std::string& fname);
int writeFileBinary(const std::string& fname, const std::string& data);

#endif