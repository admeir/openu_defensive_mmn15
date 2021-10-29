#include <iostream>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include "Client.h"

Client::Client(boost::asio::io_context& io_context) :
    sock(io_context), _io_context(io_context) {
    
}

Client::~Client() {
    disconnect();
}

void Client::connect(const char* ip, const char* port) {
    try {
        tcp::resolver resolver(_io_context);
        boost::asio::connect(sock, resolver.resolve(ip, port));
    }
    catch (std::exception& e) {
        std::cout << "while trying to connect " << ip << ":" << port << "\n";
        std::cerr << "Exception: " << e.what() << "\n";
        exit(1);
    }
}

void Client::disconnect() {
    sock.close();
}

void Client::sendMsg(std::string &data) {
    try {
        boost::asio::write(sock, boost::asio::buffer(data.c_str(), data.length()));
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

size_t Client::getMsg(boost::asio::streambuf &buf) {
    buf_len = -1;
    
    try {
        size_t readen_len = boost::asio::read_until(sock, buf, END_OF_PACKET);
        buf_len = readen_len - (sizeof(END_OF_PACKET) - 2);
        return buf_len;
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}