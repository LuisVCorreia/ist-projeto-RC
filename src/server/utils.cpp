#include "utils.hpp"


int createAuctionDir(std::string& aid){
    if (aid.length() != 3)
        return 0;

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS") / aid;
    fs::path BIDS_dir = AUCTION_dir / "BIDS";

    try {
        // Create auction directory
        if (!fs::create_directories(AUCTION_dir)) {
            std::cerr << "Directory already exists or cannot be created: " << AUCTION_dir << std::endl;
            return 0;
        }

        // Create bids directory
        if (!fs::create_directories(BIDS_dir)) {
            std::cerr << "Failed to create BIDS directory in: " << BIDS_dir << std::endl;
            return 0;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;

}



int createUserDir(std::string& uid) {
    if (uid.length() != 6)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path HOSTED_dir = USER_dir / "HOSTED";
    fs::path BIDDED_dir = USER_dir / "BIDDED";

    try {
        // Create user directory
        if (!fs::create_directories(USER_dir)) {
            std::cerr << "Directory already exists or cannot be created: " << USER_dir << std::endl;
            return 0;
        }

        // Create hosted directory
        if (!fs::create_directories(HOSTED_dir)) {
            std::cerr << "Failed to create HOSTED directory in: " << HOSTED_dir << std::endl;
            return 0;
        }

        // Create bidded directory
        if (!fs::create_directories(BIDDED_dir)) {
            std::cerr << "Failed to create BIDDED directory in: " << BIDDED_dir << std::endl;
            return 0;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;
}



// check if user directory exists
int existsUserDir(std::string& uid) {
    if (uid.length() != 6)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;

    try {
        if (!fs::exists(USER_dir))
            return 0;
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;

}


// int isUserRegistered(std::string& uid) {}


int createLogin(std::string& uid) {
    if (uid.length() != 6)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path LOGIN_file = USER_dir / (uid + "_login.txt");

    try {
        if (!fs::exists(LOGIN_file)) {
            return 0;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 1;
}



int createPassword(std::string& uid, std::string& password) {
    if (uid.length() != 6 || password.length() != 8)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path PASSWORD_file = USER_dir / (uid + "_pass.txt");

    try {
        if (!fs::exists(PASSWORD_file)) {
            std::ofstream ofs(PASSWORD_file);
            ofs << password << std::endl;
            ofs.close();
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 1;
}



int eraseLogin(std::string& uid) {
    if (uid.length() != 6)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path LOGIN_file = USER_dir / (uid + "_login.txt");

    try {
        if (fs::exists(LOGIN_file)) {
            fs::remove(LOGIN_file);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 1;
}


int checkAssetFile(std::string& fname)
{
    struct stat filestat;
    int ret_stat;

    ret_stat = stat(fname.c_str(), &filestat);

    if (ret_stat == -1 || filestat.st_size == 0)
        return (0);

    return (filestat.st_size);
}



// int getBidList(std::string&  aid, BIDLIST *list)
// {
//     struct dirent **filelist;
//     int n_entries, n_bids, len;
//     char dirname[20];
//     char pathname[32];

//     sprintf(dirname, "AUCTIONS/%03d/BIDS/", aid);
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
//             sprintf(pathname, "AUCTIONS/%03d/BIDS/%s", aid, filelist[n_entries]->d_name);
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