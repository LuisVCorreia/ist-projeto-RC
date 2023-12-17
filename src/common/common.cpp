#include "common.hpp"


void createUDPConn(int& fd) {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(1); //TODO remove all exits and handle errors
}

void closeUDPConn(int& fd) {
    close(fd);
}

void createTCPConn(int& fd, struct addrinfo *res) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) exit(1); // TODO remove all exits and handle errors
    
    int errcode = connect(fd, res->ai_addr, res->ai_addrlen);
    if (errcode == -1) exit(1); //TODO remove all exits and handle errors
}


void closeTCPConn(int& fd) {
    close(fd);
}


// bool readTCPdata(int& fd, std::string& response) {
//     char buffer[1024];
//     ssize_t n;

//     while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0 ) {     
//         // Null-terminate the received data
//         buffer[n] = '\0';

//         // Append the received data to the string
//         response.append(buffer, n);

//         if (response.back() == '\n') {
//             break;
//         }
//     }
        
//     if (n == -1){
//         perror("Error reading from socket");
//         return false; // error whilst reading
// }

//     //check last character of response
//     if (response.empty() || response.back() != '\n')
//         return false;
    
//     return true;
// }


// Validations

int loginValid(std::string& uid, std::string& password) {
    if (uid.length() == 6 && all_of(uid.begin(), uid.end(), ::isdigit)
        && password.length() == 8 && all_of(password.begin(), password.end(), ::isalnum)) {
        return 1;
    } else 
        return 0;
}

int isAidValid(std::string& aid) {
    return (aid.length() == 3 && std::all_of(aid.begin(), aid.end(), ::isdigit));
}


int isFnameValid(std::string& fname) {
    // check fname is limited to a total of 24 alphanumerical characters 
    // (plus ‘-‘, ‘_’ and ‘.’), including the separating dot and the 3-letter extension: “nnn…nnnn.xxx”.

    auto len = fname.length();
    if (len <= 24 && len >= 5 && fname[len - 4] == '.') {

        auto isValidChar = [](char c) { // lambda function
            return std::isalnum(c) || c == '-' || c == '_' || c == '.';
        };

        if (!(std::all_of(fname.begin(), fname.end() - 4, isValidChar)) || 
            !(std::all_of(fname.end() - 3, fname.end(), ::isalnum))) {
            std::cout << "Invalid Fname format" << std::endl;
            return 0; 
        }
    } else {
        std::cout << "Invalid Fname format" << std::endl;
        return 0;
    }
    return 1;
}


int isAuctionNameValid(std::string& name) {
    if (name.length() > 10 || name.length() == 0 ||
        !std::all_of(name.begin(), name.end(), ::isalnum)) {
        std::cout << "Invalid description name" << std::endl;
        return 0;
    }
    return 1;
}


int isValueValid(std::string& value) {
    if (value.length() > 6 || 
        !std::all_of(value.begin(), value.end(), ::isdigit)) {
        std::cout << "Invalid start value" << std::endl;
        return 0;
    }
    return 1;
}


int isTimeActiveValid(std::string& timeactive) {
    // check timeactive is represented with up to 5 digits
    if (timeactive.length() > 5 || 
        !std::all_of(timeactive.begin(), timeactive.end(), ::isdigit)) {
        std::cout << "Invalid duration of auction" << std::endl;
        return 0;
    }
    return 1;
}

int isFsizeValid(const std::string& fsize) {
    // check fsize is represented with up to 8 digits
    if (fsize.length() > 8 || 
        !std::all_of(fsize.begin(), fsize.end(), ::isdigit) ||
        std::stoi(fsize) > 1024 * 1024 * 10 || std::stoi(fsize) < 0) {
        std::cout << "Invalid file size" << std::endl;
        return 0;
    }
    return 1;
}


// Binary Files


std::string readFileBinary(const std::string& fname) {
    std::ifstream file(fname, std::ios::binary);
    if (!file) {
        std::cout << "Cannot open file: " << fname << std::endl;
    }

    std::ostringstream oss;
    oss << file.rdbuf(); // read the file

    if (!oss) {
        std::cout << "Failed to read file: " << fname << std::endl;
    }
    return oss.str();
}


int writeFileBinary(const std::string& fname, const std::string& data) {
    std::ofstream file(fname, std::ios::binary | std::ios::out);
    if (!file) {
        std::cout << "Cannot open file for writing: " << fname << std::endl;
        return false;
    }

    // Write the binary data to the file
    file.write(data.c_str(), data.size());

    if (!file.good()) {
        std::cout << "Failed to write file: " << fname << std::endl;
        return false;
    }

    // Flush the output stream and close the file
    file.flush();
    file.close();
    return true;
}