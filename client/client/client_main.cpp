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
    boost::asio::io_context io_context;
    MessageUClient client(io_context);
    try
    {
        client.connect();
        client.cli();
        client.disconnect();
    }
    catch (std::exception& e)
    {
        client.disconnect();
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
