#pragma once

#define CLI_CMD_MAX_LEN 2

#define  PRINT_CLI_INSTRACTION() do{ \
std::cout << \
"MessageU client at your service.\n" << \
"10) Register\n" << \
"20) Request for clients list\n" << \
"30) Request for public key\n" << \
"40) Request for waiting messages\n" << \
"50) Send a text message\n" << \
"51) Send a request for symmetric key\n" << \
"52) Send your symmetric key\n" << \
"0) E\n" << \
std::endl; \
} while(0)

class MessageUClient : public Client {
public:
    MessageUClient(boost::asio::io_context& io_context);
    virtual ~MessageUClient() {};
    void connect();
    void cli();

private:
    OPERATION_STATUS initKeys();
    OPERATION_STATUS initKeys(std::string& encoded_key);
    OPERATION_STATUS execCMD(unsigned short int cmd);
    OPERATION_STATUS checkCode(uint16_t code);
    void initReqHeader();
    void sendReq();
    OPERATION_STATUS reciveRespHeader(boost::asio::streambuf &buf);
    OPERATION_STATUS reciveRespPayload(boost::asio::streambuf &buf, char* payload);
    OPERATION_STATUS sendRegistrationReq();
    OPERATION_STATUS recvRegistrationResp(char *resp_payload);
    OPERATION_STATUS regitrete();
    OPERATION_STATUS recvClientsListResp();
    OPERATION_STATUS getClientsList();
    OPERATION_STATUS getClientsPublicKey(clientId &id, clientPublicKey *publicKey);
    OPERATION_STATUS getClientsPublicKey();
    bool got_exit = false;
    bool me_info_initilaized = false;
    unsigned char aes_key[AESWrapper::DEFAULT_KEYLENGTH];
    
    uint8_t version = 1;
    MeInfoFileRWIF me_info;
    AESWrapper aes_wrapper;
    std::string pub_key;
    std::string prv_key;
    std::string base64key;
    MUPReqMessage req;
    MUPRespMessage resp;
};
