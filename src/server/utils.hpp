#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>

#include <filesystem>

namespace fs = std::filesystem;

#define MAX_BIDS 50


typedef struct {
    char uid[7];
    char value[7];
    char datetime[20];
    time_t sec_time;
    
} BID;

typedef struct {
    int no_bids;
    BID bids[MAX_BIDS];

} BIDLIST;

int createAuctionDir(std::string& aid);
int createUserDir(std::string& uid);
int existsUserDir(std::string& uid);
int createLogin(std::string& uid);
int createPassword(std::string& uid, std::string& password);
int eraseLogin(std::string& uid);
int checkAssetFile(std::string& fname);

//int GetBidList(std::string& aid, BIDLIST *list);


#endif