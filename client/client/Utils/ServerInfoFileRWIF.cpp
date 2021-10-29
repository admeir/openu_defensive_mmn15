#include "ServerInfoFileRWIF.h"
#include "Utils.h"

ServerInfoFileRWIF::ServerInfoFileRWIF(): FileRWIF("server.info") {
    EXIT_ON_FAIL(parseData());
}

std::string ServerInfoFileRWIF::getIP() {
    return ip;
};

std::string ServerInfoFileRWIF::getPort() {
    return port;
};

FILE_ACCESS_STATUS ServerInfoFileRWIF::parseData() {
    std::string delimiter = ":";
    FILE_ACCESS_STATUS status = readFile();
    if (OPERATION_STATUS_SUCCESS != validate(readen_data, delimiter)) {
        std::cerr << "ERROR: file sever info must include <ip>:<port>" << std::endl;
        status = FILE_ACCESS_STATUS_FAIL;
    }
    if (FILE_ACCESS_STATUS_SUCCESS == status) {
        splitedString file_params = split(readen_data, delimiter);
        if (SERVER_INFO_POSS_NUM_OF_PARAMS != file_params.size()) {
            std::cerr << "ERROR: file sever info must include <ip>:<port>" << std::endl;
            return FILE_ACCESS_STATUS_FAIL;
        }
        ip = file_params[SERVER_INFO_POSS_IP];
        port = file_params[SERVER_INFO_POSS_PORT];
        closeFile();
    } else {
        std::cerr << "ERROR: canot open file: " << file_path << std::endl;
        status = FILE_ACCESS_STATUS_FAIL;
    }
    if (!isValidIpAddress(ip)) { 
        std::cerr << "ERROR: Bad IP" << ip << std::endl; 
        status = FILE_ACCESS_STATUS_FAIL;
    }
    if (!isNumber(port)) { 
        std::cerr << "ERROR: Bad Port" << port << std::endl; 
        status = FILE_ACCESS_STATUS_FAIL;
    }
    if (FILE_ACCESS_STATUS_SUCCESS == status) {
        parsed = true;
    }
    return status;
}