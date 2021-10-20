#include <string>

#include "Utils/Utils.h"
#include "Utils/MeInfoFileRWIF.h"
#include "Utils/ServerInfoFileRWIF.h"
#include <iostream>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;
#include "Client.h"
#include "cryptopp_wrapper/Base64Wrapper.h"
#include "cryptopp_wrapper/AESWrapper.h"
#include "cryptopp_wrapper/RSAWrapper.h"
#include "../../protocol/MessageUProtocolReq.h"
#include "../../protocol/MessageUProtocolResp.h"
#include "MessageUClient.h"


int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_context io_context;
        MessageUClient client(io_context);
        client.cli();
            /*
        char reply[MAX_LENGTH];
        size_t reply_length = client.getMsg(reply);
        std::cout << "got : ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";

        std::cout << "Enter message: ";
        char request[MAX_LENGTH];
        std::cin.getline(request, MAX_LENGTH);
        size_t request_length = std::strlen(request);
        client.sendMsg(request, request_length);

        */


    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
