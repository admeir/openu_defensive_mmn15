#include <string>

#include "Utils/Utils.h"
#include "Utils/MeInfoFileRWIF.h"
#include "Utils/ServerInfoFileRWIF.h"

#include "cryptopp_wrapper/Base64Wrapper.h"
#include "cryptopp_wrapper/AESWrapper.h"
#include "cryptopp_wrapper/RSAWrapper.h"
#include "../../protocol/MessageUProtocolReq.h"
#include "../../protocol/MessageUProtocolResp.h"

#include <iostream>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;
#include "Client.h"
#include "MessageUClient.h"

MessageUClient::MessageUClient(boost::asio::io_context& io_context) :
    Client(io_context),
    me_info(),
    aes_wrapper(AESWrapper::GenerateKey(aes_key, AESWrapper::DEFAULT_KEYLENGTH),
                                        AESWrapper::DEFAULT_KEYLENGTH) {

    if (me_info.exists()) {
        std::string key = me_info.getPrivateKey();
        initKeys(key);
        initReqHeader();
    }
    //RSAPublicWrapper rsa_encryptor(pub_key);
    //rsa_decryptor(base64key);


}

void MessageUClient::initReqHeader() {
    ZERRO_MEM(req.header);
    SET_MEM(req.header.id, me_info.getID().c_str());
    req.header.version = version;
}

OPERATION_STATUS MessageUClient::initKeys(std::string &encoded_key) {
    prv_key = Base64Wrapper::decode(encoded_key);
    RSAPrivateWrapper rsa_key_gen(prv_key);
    pub_key = rsa_key_gen.getPublicKey();
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::initKeys() {
    RSAPrivateWrapper rsa_key_gen;
    pub_key = rsa_key_gen.getPublicKey();
    prv_key = rsa_key_gen.getPrivateKey();
    std::string encoded_key = Base64Wrapper::encode(prv_key);
    return me_info.setPrivateKey(encoded_key);
}

void MessageUClient::connect() {
    ServerInfoFileRWIF server_info;
    Client::connect(server_info.getIP().c_str(), server_info.getPort().c_str());
}

void MessageUClient::sendReq() {
    std::string header((char*)&(req.header), sizeof(req.header));
    std::string payload(req.payload, req.header.payload_size);
    std::string packet = header + payload;
    sendMsg(packet);
}

OPERATION_STATUS MessageUClient::reciveRespHeader(boost::asio::streambuf &buf) {
    resp.header.code = MUP_RESP_MESSAGE_CODE_ERR;
    Client::getMsg(buf);
    const char* got_data = boost::asio::buffer_cast<const char*>(buf.data());
    memcpy((char*)&resp.header, got_data, sizeof(resp.header));
    RETURN_ON_FAIL(checkCode(resp.header.code));

    if (buf_len != (resp.header.payload_size + sizeof(resp.header))) {
        std::cerr << "server responded with an error" << std::endl;
    }
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::reciveRespPayload(boost::asio::streambuf &buf, char* payload){
    const char* got_data = boost::asio::buffer_cast<const char*>(buf.data());
    memcpy(payload, &got_data[sizeof(resp.header)],resp.header.payload_size);
    resp.payload = (MUPPayload)payload;
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::checkCode(uint16_t code) {
    switch (code) {
    case MUP_RESP_MESSAGE_CODE_REGISTRETION_SUCCESSED:
    case MUP_RESP_MESSAGE_CODE_CLIENTS_LIST:
    case MUP_RESP_MESSAGE_CODE_GET_PUBLIC_KEY:
    case MUP_RESP_MESSAGE_CODE_MESSAGE_SENT:
        return OPERATION_STATUS_SUCCESS;
        break;
    default:
        std::cerr << "server responded with an error" << std::endl;
    }
    return OPERATION_STATUS_FAIL;
}

void MessageUClient::cli() {
    std::string cli_buffer;
    do {
        PRINT_CLI_INSTRACTION();
        std::getline(std::cin, cli_buffer);

        if (isNumber(cli_buffer.substr(0, CLI_CMD_MAX_LEN))) {
            execCMD(atoi(cli_buffer.substr(0, CLI_CMD_MAX_LEN).c_str()));
        } else {
            std::cerr << "ERROR: Wrong cmd: " << cli_buffer.substr(0, CLI_CMD_MAX_LEN) \
                << "," << std::endl <<  "please follow instuction!" << std::endl;
        }
    } while (!got_exit);
}

OPERATION_STATUS MessageUClient::sendRegistrationReq() {
    RETURN_ON_FAIL(me_info.updateNameFromCli());
    initReqHeader();
    req.header.code = MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION;
    MUPReqRegistretionPayload req_payload;
    req.payload = (char*)&req_payload;
    req.header.payload_size = sizeof(req_payload);
    ZERRO_MEM(req_payload);
    SET_MEM(req_payload.name, me_info.getName().c_str());
    RETURN_ON_FAIL(initKeys());
    SET_MEM(req_payload.public_key, pub_key.c_str());
    sendReq();
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::recvRegistrationResp(char *resp_payload) {
    boost::asio::streambuf buf;
    RETURN_ON_FAIL(reciveRespHeader(buf));
    RETURN_ON_FAIL(reciveRespPayload(buf, resp_payload));
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::regitrete() {
    if (me_info.exists()) {
        std::cerr << "ERROR Not Valid Command: me.info allredy exists." << std::endl;
        return OPERATION_STATUS_FAIL;
    }
    RETURN_ON_FAIL(sendRegistrationReq());
    MUPRespRegistretionSuccessedPayload resp_payload;
    RETURN_ON_FAIL(recvRegistrationResp((char *)&resp_payload));
    std::string id_str(resp_payload.id.id, sizeof(resp_payload.id.id));
    SET_MEM(req.header.id.id, resp_payload.id.id);
    RETURN_ON_FAIL(me_info.setID(id_str));
    RETURN_ON_FAIL(me_info.writeData());
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::recvClientsListResp() {
    boost::asio::streambuf buf;
    MUPReqResClientsPayload payload;
    RETURN_ON_FAIL(reciveRespHeader(buf));
    int num_of_clients = resp.header.payload_size/ sizeof(MUPReqResClientStruct);
    MUPReqResClientStruct *clients = new MUPReqResClientStruct[num_of_clients];
    
    RETURN_ON_FAIL(reciveRespPayload(buf, (char*)clients));
    payload.clients = clients;
    std::cout << "Clients list (" << num_of_clients  << "): " << std::endl;
    for (int i = 0; i < num_of_clients; i++) {
        std::string ID(payload.clients[i].id.id, sizeof(payload.clients[0].id.id));
        std::cout << "\tID: " << ID\
                  << " Name: " << payload.clients[i].name.name << std::endl;
    }
    delete[] payload.clients;
    
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::getClientsList() {
    req.header.code = MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST;
    req.header.payload_size = 0;
    sendReq();
    recvClientsListResp();
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::getClientsPublicKey(clientId &id, clientPublicKey *publicKey) {
    MUPReqGetPublicKeyPayload req_payload;
    req.header.code = MUP_REQ_MESSAGE_COD_TYPE_GET_PUBLIC_KEY;
    req.header.payload_size = sizeof(req_payload);
    req.payload = (char*)&req_payload;
    SET_MEM(req_payload.id, &id);
    sendReq();
    MUPRespGetPublicKeyPayload resp_payload;
    RETURN_ON_FAIL(recvRegistrationResp((char*)&resp_payload));
    std::string recev_id_str(resp_payload.id.id, sizeof(resp_payload.id.id));
    std::string got_id_str(id.id, sizeof(id.id));
    if (recev_id_str.compare(got_id_str) != 0) {
        return OPERATION_STATUS_FAIL;
    }
    SET_MEM(publicKey->public_key, &resp_payload.publicKey);
        
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::getClientsPublicKey() {
    std::cout << "please enter client id: ";
    std::string readen_id;
    clientId id;
    clientPublicKey publicKey;
    std::getline(std::cin, readen_id);
    SET_MEM(id.id, readen_id.c_str());
    RETURN_ON_FAIL(getClientsPublicKey(id, &publicKey));
    std::string receved_public_key(publicKey.public_key, sizeof(publicKey.public_key));
    std::cout << "receved public key: " << receved_public_key << std::endl;
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::sendMsgClient() {
    std::string readen_name;
    std::string readen_msg;
    std::cout << "please enter client name: ";
    std::getline(std::cin, readen_name);
    std::cout << "please enter message: ";
    std::getline(std::cin, readen_name);
    SET_MEM(id.id, readen_id.c_str());
    RETURN_ON_FAIL(getClientsPublicKey(id, &publicKey));
    std::string receved_public_key(publicKey.public_key, sizeof(publicKey.public_key));
    std::cout << "receved public key: " << receved_public_key << std::endl;
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::execCMD(unsigned short int cmd) {
    connect();
    switch (cmd) {
    case 10:
        regitrete();
        break;
    case 20:
        getClientsList();
        break;
    case 30: 
        getClientsPublicKey();
        break;
    case 40:

        break;
    case 50:

        break;
    case 51:

        break;
    case 52:

        break;
    case 0:
        got_exit = true;
        break;
    default:
        std::cerr << "ERROR: Wrong cmd: " << cmd \
            << "," << std::endl << "please follow instuction!" << std::endl;
    }
    disconnect();
    return OPERATION_STATUS_SUCCESS;
}