#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

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

int CreateAUCTIONDir(int AID);
int CreateLogin(char *UID);
int EraseLogin(char *UID);
int CheckAssetFile(char *fname);
//int GetBidList(int AID, BIDLIST *list);


#endif