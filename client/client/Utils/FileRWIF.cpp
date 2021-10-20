#include "FileRWIF.h"

FileRWIF::FileRWIF(const char *path): file_path(path) {
}

FileRWIF::~FileRWIF() {
    
    closeFile();
}

bool FileRWIF::exists() {
    FILE_ACCESS_STATUS status = openFile();
    if (FILE_ACCESS_STATUS_SUCCESS == status) {
        exist = true;
        closeFile();
    } 
    return exist;
}

bool FileRWIF::parseSucceed() {
    return parsed;
}

FILE_ACCESS_STATUS FileRWIF::openFile() {
    file_ptr.open(file_path.c_str(), std::fstream::in | std::fstream::out);
    if (file_ptr.is_open()) {
        return FILE_ACCESS_STATUS_SUCCESS;
    }
    return FILE_ACCESS_STATUS_FAIL;
}


void FileRWIF::closeFile() {
    if (file_ptr.is_open()) {
        file_ptr.close();
    }
}

FILE_ACCESS_STATUS FileRWIF::readFile(){
    FILE_ACCESS_STATUS status = openFile();
    std::string word;
    if (FILE_ACCESS_STATUS_SUCCESS == status) {
        file_ptr.seekg(0, std::ios::end);
        size_t size = file_ptr.tellg();
        readen_data.resize(size, ' ');
        file_ptr.seekg(0);
        file_ptr.read(&readen_data[0], size);
        closeFile();
    }
    return status;
}

FILE_ACCESS_STATUS FileRWIF::writeFile(std::string data) {
    std::ofstream writer(file_path);
    if (writer.is_open()) {
        writer << data;
        writer.close();
        return FILE_ACCESS_STATUS_SUCCESS;
    }
    return FILE_ACCESS_STATUS_FAIL;
}

