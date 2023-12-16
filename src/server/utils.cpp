#include "utils.hpp"


// User Directory

// create new user directory
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


// check if the user directory exists
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

int createNewHost(std::string& uid, std::string& aid) {
    if (uid.length() != 6 || aid.length() != 3)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path HOSTED_dir = USER_dir / "HOSTED";
    fs::path AID_file = HOSTED_dir / (aid + ".txt");

    try {
        if (!fs::exists(HOSTED_dir))
            return 0;
        std::ofstream ofs(AID_file);
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;
}

int checkAuctionOwner(std::string& uid, std::string& aid){
    if (uid.length() != 6 || aid.length() != 3)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path HOSTED_dir = USER_dir / "HOSTED";
    fs::path AID_file = HOSTED_dir / (aid + ".txt");

    try {
        if (!fs::exists(AID_file))
            return 0;
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;

}


std::string getAuctionHost(std::string& aid){
    if (aid.length() != 3)
        return "";

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS/") / aid;
    fs::path START_file = AUCTION_dir / ("/START_" + aid + ".txt");

    try {
        if (!fs::exists(START_file)) {
            std::ifstream file(START_file);
            file >> aid;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return ""; //TODO check for this on server code
    }
    return aid;

}


std::string getAuctionStartFullTime(std::string& aid){
    std::string start_fulltime;

    if (aid.length() != 3)
        return "";

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS/") / aid;
    fs::path START_file = AUCTION_dir / ("/START_" + aid + ".txt");

    try {
        if (!fs::exists(START_file)) {
            std::ifstream file(START_file);
            std::string uid, name, fname, start_value, timeactive, start_date, start_time;
            file >> uid >> name >> fname >> start_value >> timeactive >> start_date >> start_time >> start_fulltime;

            return start_fulltime;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return "";
    }
    return start_fulltime;

}


// Password


// create new password file
int createPassword(std::string& uid, std::string& password) {
    if (uid.length() != 6 || password.length() != 8)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path PASSWORD_file = USER_dir / (uid + "_pass.txt");

    try {
        if (!fs::exists(PASSWORD_file)) {
            std::ofstream ofs(PASSWORD_file);
            ofs << password;
            ofs.close();
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 1;
}


// erase password file
int erasePassword(std::string& uid) {
    if (uid.length() != 6)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path PASSWORD_file = USER_dir / (uid + "_pass.txt");

    try {
        if (fs::exists(PASSWORD_file)) {
            fs::remove(PASSWORD_file);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 1;
}


// check if the user is registered
int isUserRegistered(std::string& uid) {
    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path PASSWORD_file = USER_dir / (uid + "_pass.txt");

    try {
        if (!fs::exists(PASSWORD_file))
            return 0;
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;
}

// check that the password is correct
int isValidPassword(std::string& uid, std::string& password){
    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path PASSWORD_file = USER_dir / (uid + "_pass.txt");

    try {
        if (!fs::exists(PASSWORD_file))
            return 0;
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    std::ifstream ifs(PASSWORD_file);
    std::string correctPassword;
    ifs >> correctPassword;
    ifs.close();

    if (password != correctPassword){
        return 0;
    }

    return 1;
}


// Login


// create new login file
int createLogin(std::string& uid) {
    if (uid.length() != 6)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path LOGIN_file = USER_dir / (uid + "_login.txt");

    try {
        if (!fs::exists(LOGIN_file)) 
            std::ofstream ofs(LOGIN_file);
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;

}


// erase login file
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

// calls the eraseLogin method for each user in the server
int logoutAllUsers() {
    fs::path usersDir = fs::path("src/server/USERS");

    if (!fs::exists(usersDir)) {
        std::cerr << "Users directory not found" << std::endl;
        return 0;
    }

    try {
        for (const auto& entry : fs::directory_iterator(usersDir)) {
            if (fs::is_directory(entry)) {
                std::string uid = entry.path().filename().string();
                eraseLogin(uid);
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;
}


// check if the user is logged in
int isUserLogged(std::string& uid) {
    if (uid.length() != 6)
        return 0;

    fs::path USER_dir = fs::path("src/server/USERS") / uid;
    fs::path LOGIN_file = USER_dir / (uid + "_login.txt");

    try {
        if (fs::exists(LOGIN_file)) {
            return 1;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 0;
}


// Auction

int getNumAuctions() {
    fs::path AUCTIONS_dir = fs::path("src/server/AUCTIONS");

    if (!fs::exists(AUCTIONS_dir)) {
        std::cerr << "Auctions directory not found" << std::endl;
        return 0;
    }

    int numAuctions = 0;

    try {
        for (const auto& entry : fs::directory_iterator(AUCTIONS_dir)) {
            if (fs::is_directory(entry)) {
                numAuctions++;
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return numAuctions;
}


int existsAuctionDir(std::string& aid) {
    if (aid.length() != 3)
        return 0;

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS") / aid;

    try {
        if (!fs::exists(AUCTION_dir))
            return 0;
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;
}


// create new auction directory
int createAuctionDir(std::string& aid){
    if (aid.length() != 3)
        return 0;

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS") / aid;
    fs::path BIDS_dir = AUCTION_dir / "BIDS";
    fs::path ASSET_dir = AUCTION_dir / "ASSET";

    try {
        // create auction directory
        if (!fs::create_directories(AUCTION_dir)) {
            std::cerr << "Directory already exists or cannot be created: " << AUCTION_dir << std::endl;
            return 0;
        }

        // create bids directory
        if (!fs::create_directories(BIDS_dir)) {
            std::cerr << "Failed to create BIDS directory in: " << BIDS_dir << std::endl;
            return 0;
        }

        // create asset directory
        if (!fs::create_directories(ASSET_dir)) {
            std::cerr << "Failed to create ASSET directory in: " << ASSET_dir << std::endl;
            return 0;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;

}


// create start file in auction directory
int createStartFile(std::string& aid, std::string& uid, std::string& name, std::string& fname, std::string& start_value, std::string& timeactive){
    if (aid.length() != 3)
        return 0;

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS") / aid;
    fs::path START_file = AUCTION_dir / ("START_" + aid + ".txt");

    //get current time

    time_t now = time(0); // gets time in seconds since 1970
    tm* timeinfo = localtime(&now); // gets time in struct tm format

    char buffer[80]; //TODO adjust size
    // converts to string in format YYYY-MM-DD HH:MM:SS
    strftime(buffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);

    std::string start_datetime(buffer); // in date format
    std::string start_fulltime = std::to_string(now); // in seconds


    try {
        if (!fs::exists(START_file)) {
            std::ofstream ofs(START_file);
            ofs << uid << " " << name << " " << fname << " " << start_value << " " << timeactive
                << " " << start_datetime << " " << start_fulltime;
            ofs.close();
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 1;
}


// creates end file containing end_datetime and end_sec_time where end_datetime is the date 
// in format YYYY-MM-DD HH:MM:SS and end_sec_time is the number of seconds in which the auction was active for
int createEndFile(std::string& aid, std::string& end_datetime, std::string& end_sec_time){
    if (aid.length() != 3)
        return 0;

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS") / aid;
    fs::path END_file = AUCTION_dir / ("END_" + aid + ".txt");

    try {
        if (!fs::exists(END_file)) {
            std::ofstream ofs(END_file);
            ofs << end_datetime << " " << end_sec_time;
            ofs.close();
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 1;
}



int isAuctionStillActive(std::string& aid){
    if (aid.length() != 3)
        return 0;

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS") / aid;
    fs::path START_file = AUCTION_dir / ("START_" + aid + ".txt");
    fs::path END_file = AUCTION_dir / ("END_" + aid + ".txt");

    try {
        if (!fs::exists(START_file) || fs::exists(END_file)) {
            return 0;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    std::ifstream ifs(START_file);
    std::string uid, name, fname, start_value, timeactive, start_date, start_time, start_fulltime;
    ifs >> uid >> name >> fname >> start_value >> timeactive >> start_date >> start_time >> start_fulltime;
    ifs.close();

    // check whether start_datetime + start_fulltime is greater than current time since 1970
    long time_active_seconds = std::stol(timeactive);
    time_t start_time_seconds = std::stol(start_fulltime);
    time_t now = time(0);

    if (now - start_time_seconds > time_active_seconds) {
        // auction is no longer active

        // end_datetime is the start time plus the time active
        time_t end_time_seconds = start_time_seconds + time_active_seconds;
        tm* end_timeinfo = localtime(&end_time_seconds);
        // TODO: make this part a function since it is repeated in createStartFile
        char end_buffer[80];
        strftime(end_buffer, 80, "%Y-%m-%d %H:%M:%S", end_timeinfo);

        std::string end_datetime(end_buffer);
        std::string end_sec_time = std::to_string(time_active_seconds);

        createEndFile(aid, end_datetime, end_sec_time);
        return 0;
    }
    return 1;
}



// closes the auction while it is still active
int closeActiveAuction(std::string& aid){
    if (aid.length() != 3)
        return 0;

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS") / aid;
    fs::path START_file = AUCTION_dir / ("START_" + aid + ".txt");
    fs::path END_file = AUCTION_dir / ("END_" + aid + ".txt");

    // Check if the auction is already closed
    if (fs::exists(END_file)) {
        std::cerr << "Auction already closed." << std::endl;
        return 0;
    }

    // Check if the start file exists
    if (!fs::exists(START_file)) {
        std::cerr << "Start file does not exist." << std::endl;
        return 0;
    }

    std::ifstream ifs(START_file);
    if (!ifs) {
        std::cerr << "Unable to open start file." << std::endl;
        return 0;
    }

    std::string uid, name, fname, start_value, timeactive, start_date, start_time, start_fulltime;
    ifs >> uid >> name >> fname >> start_value >> timeactive >> start_date >> start_time >> start_fulltime;
    ifs.close();

    time_t now = time(0);
    tm* timeinfo = localtime(&now);

    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

    std::string end_datetime(buffer);
    time_t start_time_seconds = std::stol(start_fulltime);
    std::string end_sec_time = std::to_string(now - start_time_seconds);

    return createEndFile(aid, end_datetime, end_sec_time);
}

// create asset file in auction directory
int createAssetFile(std::string& aid, std::string& fname, std::string& fdata){
    if (aid.length() != 3)
        return 0;

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS") / aid;
    fs::path ASSET_dir = AUCTION_dir / "ASSET";
    fs::path ASSET_file = ASSET_dir / fname;

    try {
        if (!fs::exists(ASSET_file)) {
            std::ofstream ofs(ASSET_file);
            ofs << fdata;
            ofs.close();
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 1;
}


// check if the asset file exists
int checkAssetFile(std::string& fname)
{
    struct stat filestat;
    int ret_stat;

    ret_stat = stat(fname.c_str(), &filestat);

    if (ret_stat == -1 || filestat.st_size == 0)
        return (0);

    return (filestat.st_size);
}


// Bids


// create new bid file
int placeBid(std::string& aid, std::string& uid, std::string& value){
    if (aid.length() != 3 || uid.length() != 6)
        return 0;
    
    while (value.length() < 6) {
        value = "0" + value; // pad with zeros
    }

    fs::path AUCTION_dir = fs::path("src/server/AUCTIONS") / aid;
    fs::path BIDS_dir = AUCTION_dir / "BIDS";
    fs::path BID_file = BIDS_dir / (value + ".txt");

    //get current time

    time_t now = time(0); // gets time in seconds since 1970
    tm* timeinfo = localtime(&now); // gets time in struct tm format

    char buffer[80]; //TODO adjust size
    // converts to string in format YYYY-MM-DD HH:MM:SS
    strftime(buffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);

    std::string bid_datetime(buffer); // in date format
    
    time_t auction_start = std::stol(getAuctionStartFullTime(aid));
    time_t bid_sec_time = now - auction_start;

    try {
        if (!fs::exists(BID_file)) {
            std::ofstream ofs(BID_file);
            ofs << uid << " " << value << " " << bid_datetime << " " << bid_sec_time;
            ofs.close();
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;
}


// get value of last bid
int getHighestBid(std::string& aid){

    DIR* dir = opendir(("src/server/AUCTIONS/" + aid + "/BIDS").c_str());

    if (!dir) {
        std::cerr << "Error opening directory" << std::endl;
        return -1;
    }

    std::vector<std::string> files;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) { // Only regular files
            files.push_back(entry->d_name);
        }
    }

    closedir(dir);

    // no bids were placed
    if (files.empty()) {
        // get start value
        std::ifstream ifs("src/server/AUCTIONS/" + aid + "/START_" + aid + ".txt");
        if (!ifs) {
            std::cerr << "Unable to open start file" << std::endl;
            return -1;
        }

        std::string uid, name, fname, start_value;
        ifs >> uid >> name >> fname >> start_value;

        return std::stoi(start_value);
    }

    // Sort files alphabetically
    std::sort(files.begin(), files.end());

    // last file
    std::string lastFile = files.back();

    // open file and get value
    std::ifstream ifs("src/server/AUCTIONS/" + aid + "/BIDS/" + lastFile);
    if (!ifs) {
        std::cerr << "Unable to open file " << lastFile << std::endl;
        return -1;
    }

    std::string uid, value;
    ifs >> uid >> value;
    ifs.close();

    return std::stoi(value);
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