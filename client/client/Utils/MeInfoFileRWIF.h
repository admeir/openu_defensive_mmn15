#pragma once
#include "FileRWIF.h"

enum {
    ME_INFO_PARAMS_POS_NAME,
    ME_INFO_PARAMS_POS_ID,
    ME_INFO_PARAMS_POS_PRIVATEKEY,
    ME_INFO_PARAMS_POS_NUM_PARAMS,
};

class MeInfoFileRWIF :
    public FileRWIF
{
public:
    MeInfoFileRWIF();
    virtual ~MeInfoFileRWIF() {};
    
    OPERATION_STATUS writeData();
    OPERATION_STATUS updateNameFromCli();
    //getters
    std::string getID();
    std::string getName();
    std::string getPrivateKey();
    //setters
    OPERATION_STATUS setID(std::string& id);
    OPERATION_STATUS setName(std::string& name);
    OPERATION_STATUS setPrivateKey(std::string& key);

private:
    OPERATION_STATUS parseData();
    OPERATION_STATUS validateNumOfPArams(int num);
    std::string name;
    std::string id;
    std::string private_key;
};

