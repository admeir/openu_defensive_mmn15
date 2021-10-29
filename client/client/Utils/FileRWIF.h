#pragma once

#include <iostream>
#include <fstream>
using std::ifstream;

#include "Utils.h"

typedef enum {
    FILE_ACCESS_STATUS_SUCCESS = OPERATION_STATUS_SUCCESS,
    FILE_ACCESS_STATUS_FAIL = OPERATION_STATUS_FAIL,
} FILE_ACCESS_STATUS;

class FileRWIF
{
public:
    FileRWIF() {};
    FileRWIF(const char* path);
    ~FileRWIF();
    void set_path(const char* path);
    bool exists();
    bool parseSucceed();
    FILE_ACCESS_STATUS openFile();
    FILE_ACCESS_STATUS writeFile(std::string data);
    std::string getData() {
        if (FILE_ACCESS_STATUS_SUCCESS != readFile()) {
            std::cout << "canot open file " << file_path << std::endl;
        }
        return readen_data;
    }
    
protected:
    FILE_ACCESS_STATUS readFile();
    void closeFile();
    std::fstream file_ptr;
    std::string file_path;
    std::string readen_data;
    bool parsed = false;
    bool exist = false;
};

