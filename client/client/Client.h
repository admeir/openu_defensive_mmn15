#pragma once

#define END_OF_PACKET "MEIR_END_OF_PACKET\0"
#define MAX_LENGTH 1024


class Client {
public:
    Client(boost::asio::io_context& io_context);
    virtual ~Client();
    void connect(const char* ip, const char* port);
    void disconnect();
protected:
    void sendMsg(std::string& data);
    size_t getMsg(boost::asio::streambuf& buf);
    int buf_len;
private:
    boost::asio::io_context& _io_context;
    tcp::socket sock;
    const char* ip;
    const char* port;
};

