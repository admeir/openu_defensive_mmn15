#include "Utils.h"
#include <iostream>
#include <iomanip>

bool isNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool isValidIpAddress(std::string &ipAddress)
{
    boost::system::error_code ec;
    boost::asio::ip::address::from_string(ipAddress, ec);
    if (ec) {
        std::cerr << ec.message() << std::endl;
        return false;
    }
    return true;
}


void hexify(const unsigned char* buffer, unsigned int length)
{
    std::ios::fmtflags f(std::cout.flags());
    std::cout << std::hex;
    for (size_t i = 0; i < length; i++)
        std::cout << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]) << (((i + 1) % 16 == 0) ? "\n" : " ");
    std::cout << std::endl;
    std::cout.flags(f);
}

bool string_is_hex(const std::string& input)
{
    std::string hex_digits = "0123456789ABCDEFabcdef";
    std::string output;
    output.reserve(input.length() * 2);
    for (unsigned char c : input)
    {
        if (std::string::npos == hex_digits.find(c))
            return false;
    }
    return true;
}

OPERATION_STATUS validate(std::string &text, std::string &delimiter) {
    std::size_t id = text.find(delimiter);
    if (0 > id) {
        return OPERATION_STATUS_FAIL;
    }
    return OPERATION_STATUS_SUCCESS;
}

splitedString split(std::string &text, std::string &delimiter) {
    splitedString ret_vector;
    std::size_t word_size;
    std::size_t end = text.find(delimiter);

    do {
        end = text.find(delimiter);
        if (std::string::npos != end) {
            ret_vector.push_back(text.substr(0, end));
        } else {
            // without EOF
            ret_vector.push_back(text.substr(end + 1, text.length()));
        }
        text = text.substr(end+1, text.length());
    } while (std::string::npos != end);

    return ret_vector;
}

std::string join(splitedString str_list, std::string& delimiter) {
    std::string ret;
    for (std::string s: str_list) {
        ret += s + delimiter.c_str();
    }
    return ret;
}