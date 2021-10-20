#include "MeInfoFileRWIF.h"

MeInfoFileRWIF::MeInfoFileRWIF() : FileRWIF("me.info") {
    parseData();
}

std::string MeInfoFileRWIF::getID() {
    return id;
};

std::string MeInfoFileRWIF::getName() {
    return name;
};
std::string MeInfoFileRWIF::getPrivateKey() {
    return private_key;
};

OPERATION_STATUS MeInfoFileRWIF::setID(std::string &new_id) {
    if ((!string_is_hex(new_id)) || (new_id.length() != ID_SIZE)) {
        std::cerr << "ERROR: file me.info have bad id: " << new_id << std::endl;
        return OPERATION_STATUS_FAIL;
    }
    id = new_id;
    return OPERATION_STATUS_SUCCESS;
};

OPERATION_STATUS MeInfoFileRWIF::setName(std::string &new_name) {
    if (new_name.length() > MAX_NAEM_SIZE) {
        std::cerr << "ERROR: file me.info have bad name: " << new_name << std::endl;
        return OPERATION_STATUS_FAIL;
    }
    name = new_name;    
    name.resize(MAX_NAEM_SIZE);
    return OPERATION_STATUS_SUCCESS;
};

OPERATION_STATUS MeInfoFileRWIF::updateNameFromCli() {
    std::cout << "please enter ypur name message: ";
    std::string readen_name;
    std::getline(std::cin, readen_name);
    return setName(readen_name);
}

OPERATION_STATUS MeInfoFileRWIF::setPrivateKey(std::string &key) {
    private_key = key;
    return OPERATION_STATUS_SUCCESS;
};

OPERATION_STATUS MeInfoFileRWIF::validateNumOfPArams(int num) {
    if (ME_INFO_PARAMS_POS_NUM_PARAMS > num) {
        return OPERATION_STATUS_FAIL;
    }
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MeInfoFileRWIF::parseData() {
    std::string delimiter = "\n";
    
    if (FILE_ACCESS_STATUS_SUCCESS == readFile()) {
        RETURN_ON_FAIL_WITH_MSG(validate(readen_data, delimiter), "ERROR: file me.info must include: \nname\nid\nprivate_key");
        splitedString file_params = split(readen_data, delimiter);
        RETURN_ON_FAIL_WITH_MSG(validateNumOfPArams(file_params.size()), "ERROR: file me.info must include: \nname\nid\nprivate_key");
        RETURN_ON_FAIL(setName(file_params[ME_INFO_PARAMS_POS_NAME]));
        RETURN_ON_FAIL(setID(file_params[ME_INFO_PARAMS_POS_ID]));
        
        splitedString::iterator start = file_params.begin() + ME_INFO_PARAMS_POS_PRIVATEKEY;
        splitedString::iterator end = file_params.end()-1;
        splitedString splited_k(start, end);
        std::string k = join(splited_k, delimiter);
        RETURN_ON_FAIL(setPrivateKey(k));
        closeFile();
    }
    parsed = true;
	return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MeInfoFileRWIF::writeData() {
    readen_data = name + '\n';
    readen_data += id + '\n';
    readen_data += private_key;
    if (FILE_ACCESS_STATUS_SUCCESS == writeFile(readen_data)) {
        return parseData();
    }
    return OPERATION_STATUS_FAIL;
}