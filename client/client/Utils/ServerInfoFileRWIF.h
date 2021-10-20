#pragma once
#include "FileRWIF.h"

enum {
    SERVER_INFO_POSS_IP,
    SERVER_INFO_POSS_PORT,
    SERVER_INFO_POSS_NUM_OF_PARAMS,
};

class ServerInfoFileRWIF :
    public FileRWIF
{
public:
    ServerInfoFileRWIF();
    virtual ~ServerInfoFileRWIF() {};
    FILE_ACCESS_STATUS parseData();
    std::string getIP();
    std::string getPort();

private:
    std::string ip;
    std::string port;
};

