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
#include <vector>
#include <algorithm>

#include <filesystem>
#include <common/common.hpp>

namespace fs = std::filesystem;

#define MAX_BIDS 50

typedef struct {
    std::string host_uid;
    std::string auction_name;
    std::string asset_fname;
    std::string start_value;
    std::string start_datetime;
    std::string time_active;
} AuctionGeneralInfo;

typedef struct {
    std::string bidder_uid;
    std::string bid_value;
    std::string bid_datetime;
    std::string bid_sec_time;
} BidInfo;

typedef struct {
    int bid_count;
    BidInfo bids[MAX_BIDS];
} BidList;

typedef struct {
    std::string end_datetime;
    std::string end_sec_time;
} AuctionEndInfo;

int createUserDir(std::string& uid);
int existsUserDir(std::string& uid);
int createNewHost(std::string& uid, std::string& aid);
int checkAuctionOwner(std::string& uid, std::string& aid);
std::string getAuctionHost(std::string& aid);
std::string getAuctionStartFullTime(std::string& aid);

int createPassword(std::string& uid, std::string& password);
int erasePassword(std::string& uid);
int isUserRegistered(std::string& uid);
int isValidPassword(std::string& uid, std::string& password);

int createLogin(std::string& uid);
int eraseLogin(std::string& uid);
int logoutAllUsers();
int isUserLogged(std::string& uid);

int getNumAuctions();
std::string getAllAuctions();
std::string getMyAuctions(std::string& uid);
int existsAuctionDir(std::string& aid);
int createAuctionDir(std::string& aid);
int createStartFile(std::string& aid, std::string& uid, std::string& name, std::string& fname,
                    std::string& start_value, std::string& timeactive);
int createEndFile(std::string& aid, std::string& end_datetime, std::string& end_sec_time);
int isAuctionStillActive(std::string& aid);
int closeActiveAuction(std::string& aid);

int createAssetFile(std::string& aid, std::string& fname, std::string& fdata);
int getAssetFile(std::string& aid, std::string& fname, std::string& fsize, std::string& fdata);

int createNewBidder(std::string& aid, std::string& uid);
int placeBid(std::string& aid, std::string& uid, std::string& value);
int getHighestBid(std::string& aid);
std::string getMyBids(std::string& uid);

int getAuctionGeneralInfo(std::string& aid, AuctionGeneralInfo& generalInfo);
int getBidList(std::string& aid, BidList& list);
int loadBid(char *pathname, BidList& list, int n_bids);
int getAuctionEndInfo(std::string& aid, AuctionEndInfo& endInfo);


#endif