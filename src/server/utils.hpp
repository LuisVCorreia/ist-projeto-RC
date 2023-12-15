#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <fstream>

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

int createUserDir(std::string& uid);
int existsUserDir(std::string& uid);

int createPassword(std::string& uid, std::string& password);
int erasePassword(std::string& uid);
int isUserRegistered(std::string& uid);
int isValidPassword(std::string& uid, std::string& password);

int createLogin(std::string& uid);
int eraseLogin(std::string& uid);
int isUserLogged(std::string& uid);

int createAuctionDir(std::string& aid);
int createStartFile(std::string& aid, std::string& uid, std::string& name, std::string& fname,
                    std::string& start_value, std::string& timeactive);
int createAssetFile(std::string& aid, std::string& fname, std::string& fdata);
int checkAssetFile(std::string& fname);

//int GetBidList(std::string& aid, BIDLIST *list);


#endif