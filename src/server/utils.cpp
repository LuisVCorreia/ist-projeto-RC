#include "utils.hpp"


int CreateAUCTIONDir(int AID) {
    char AID_dirname[15];
    char BIDS_dirname[20];
    int ret;

    if (AID < 1 || AID > 999)
        return 0;

    sprintf(AID_dirname, "AUCTIONS/%03d", AID);
    ret = mkdir(AID_dirname, 0700);

    if (ret == -1)
        return 0;

    sprintf(BIDS_dirname, "AUCTIONS/%03d/BIDS", AID);
    ret = mkdir(BIDS_dirname, 0700);

    if (ret == -1) {
        rmdir(AID_dirname);
        return 0;
    }

    return 1;
}


int CreateLogin(char *UID) {
    char login_name[35];
    FILE *fp;

    if (strlen(UID) != 6)
        return 0;

    sprintf(login_name, "USERS/%s/%s_login.txt", UID, UID);
    fp = fopen(login_name, "w");

    if (fp == NULL)
        return 0;

    fprintf(fp, "Logged in\n");
    fclose(fp);

    return 1;
}


int EraseLogin(char *UID) {
    char login_name[35];

    if (strlen(UID) != 6)
        return 0;

    sprintf(login_name, "USERS/%s/%s_login.txt", UID, UID);
    unlink(login_name);

    return 1;
}


int CheckAssetFile(char *fname)
{
    struct stat filestat;
    int ret_stat;

    ret_stat = stat(fname, &filestat);

    if (ret_stat == -1 || filestat.st_size == 0)
        return (0);

    return (filestat.st_size);
}



// int GetBidList(int AID, BIDLIST *list)
// {
//     struct dirent **filelist;
//     int n_entries, n_bids, len;
//     char dirname[20];
//     char pathname[32];

//     sprintf(dirname, "AUCTIONS/%03d/BIDS/", AID);
//     n_entries = scandir(dirname, &filelist, 0, alphasort);
//     if (n_entries <= 0) // Could test for -1 since n_entries count always with . and ..
//         return(0);

//     n_bids=0;
//     list->no_bids=0;
//     while (n_entries--)
//     {
//         len=strlen(filelist[n_entries]->d_name);
//         if(len==10) // Discard '.', '..', and invalid filenames by size
//         {
//             sprintf(pathname, "AUCTIONS/%03d/BIDS/%s", AID, filelist[n_entries]->d_name);
//             if (LoadBid(pathname, list))
//             {
//                 ++n_bids;
//             }
//         }
//         free(filelist[n_entries]);
//         if(n_bids==50)
//             break;
//     }
//     free(filelist);
//     return(n_bids);
// }