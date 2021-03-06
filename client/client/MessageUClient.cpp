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
}

void MessageUClient::initReqHeader() {
    ZERRO_MEM(req.header);
    SET_MEM(req.header.id, me_info.getID().c_str());
    req.header.version = version;
}

OPERATION_STATUS MessageUClient::initKeys(std::string &encoded_key) {
    prv_key = Base64Wrapper::decode(encoded_key);
    RSAPrivateWrapper *x = new RSAPrivateWrapper(prv_key);
    rsapriv_wrapper = x;
    pub_key = rsapriv_wrapper->getPublicKey();
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::initKeys() {
    rsapriv_wrapper = new RSAPrivateWrapper();
    pub_key = rsapriv_wrapper->getPublicKey();
    prv_key = rsapriv_wrapper->getPrivateKey();
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
    case MUP_RESP_MESSAGE_COD_TYPE_GET_MESSAGES:
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
            execCMD((unsigned short int)atoi(cli_buffer.substr(0, CLI_CMD_MAX_LEN).c_str()));
        } else {
            std::cerr << "ERROR: Wrong cmd: " << cli_buffer.substr(0, CLI_CMD_MAX_LEN) \
                << "," << std::endl <<  "please follow instuction!" << std::endl;
        }
    } while (!got_exit);
}

OPERATION_STATUS MessageUClient::recvResp(char* resp_payload) {
    boost::asio::streambuf buf;
    RETURN_ON_FAIL(reciveRespHeader(buf));
    RETURN_ON_FAIL(reciveRespPayload(buf, resp_payload));
    return OPERATION_STATUS_SUCCESS;
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

OPERATION_STATUS MessageUClient::regitrete() {
    if (me_info.exists()) {
        std::cerr << "ERROR Not Valid Command: me.info allredy exists." << std::endl;
        return OPERATION_STATUS_FAIL;
    }
    RETURN_ON_FAIL(sendRegistrationReq());
    MUPRespRegistretionSuccessedPayload resp_payload;
    RETURN_ON_FAIL(recvResp((char *)&resp_payload));
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
    RETURN_ON_FAIL(recvResp((char*)&resp_payload));
    std::string recev_id_str(resp_payload.id.id, sizeof(resp_payload.id.id));
    std::string got_id_str(id.id, sizeof(id.id));
    if (recev_id_str.compare(got_id_str) != 0) {
        return OPERATION_STATUS_FAIL;
    }
    SET_MEM(publicKey->public_key, &resp_payload.publicKey);
        
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::saveFriendPubKey(std::string &id, std::string& pub) {
   if (frind_pub_keys.find(id) == frind_pub_keys.end()) {
       frind_pub_keys.insert(std::pair<std::string, std::string>(id, pub));
   }
   return OPERATION_STATUS_SUCCESS;
}

std::string MessageUClient::getFriendPubKey(std::string& id) {
    if (frind_pub_keys.find(id) == frind_pub_keys.end()) {
        return "";
    }
    else {
        return frind_pub_keys[id];
    }
}

OPERATION_STATUS MessageUClient::saveFriendAesKey(std::string &id, std::string &aes) {
    if (frind_aes_keys.find(id) == frind_aes_keys.end()) {
        frind_aes_keys.insert(std::pair<std::string, std::string>(id, aes));
    }
    return OPERATION_STATUS_SUCCESS;
}

std::string MessageUClient::getFriendAesKey(std::string &id) {
    if (frind_aes_keys.find(id) == frind_aes_keys.end()) {
        return "";
    }
    else {
        return frind_aes_keys[id];
    }
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
    saveFriendPubKey(readen_id, receved_public_key);
    if (receved_public_key.length() == 0) {
        return OPERATION_STATUS_FAIL;
    }
        return OPERATION_STATUS_SUCCESS;
}

std::string MessageUClient::encryptedAESKey(std::string &id_str) {
    try {
        std::string publicK = getFriendPubKey(id_str);
        if (publicK.length() == 0) {
            std::cout << "Error public key for client (" << id_str <<") - not exists" << std::endl;
            return "";
        }
        RSAPublicWrapper rsapub_wrapper(publicK);
        return rsapub_wrapper.encrypt((char*)aes_key, (unsigned int)sizeof(aes_key));
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return "";
}

std::string MessageUClient::encryptedMessage(std::string& id_str, std::string& message) {
    try {
        std::string aes_key = getFriendAesKey(id_str);
        if (aes_key.length() == 0) {
            std::cout << "Error public aes for client (" << id_str << ") - not exists" << std::endl;
            return "";
        }
        AESWrapper frined_aes_wrapper((unsigned char*)aes_key.c_str(), aes_key.length());
        return frined_aes_wrapper.encrypt(message.c_str(), message.length());
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return "";
}

OPERATION_STATUS MessageUClient::getMsgsClient() {
    unsigned int mem_ptr = 0;
    boost::asio::streambuf buf;
    MUPRespGotMessagePayload* msg_payload;
    req.header.payload_size = 0;
    req.header.code = MUP_REQ_MESSAGE_COD_TYPE_GET_MESSAGES;
    sendReq();
    RETURN_ON_FAIL(reciveRespHeader(buf));

    char* payloads = new char[resp.header.payload_size];
    RETURN_ON_FAIL(reciveRespPayload(buf, payloads));

    while (mem_ptr < resp.header.payload_size) {
        FileRWIF my_file;
        std::string file_path;
        std::string msg_content;
        msg_payload = (MUPRespGotMessagePayload*)&payloads[mem_ptr];
        std::string from_id(msg_payload->id.id, sizeof(msg_payload->id));
        msg_payload->content = &payloads[mem_ptr + sizeof(MUPRespGotMessagePayload)];
        std::string p_content_str(msg_payload->content, msg_payload->message_size);
        std::cout << "From: " << from_id << std::endl;
        switch (msg_payload->message_type)
        {
        case MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_GET_SYMETRIC_KEY:
            std::cout << "Request symmetric key" << std::endl;
            break;
        case MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_SYMETRIC_KEY:
            msg_content = rsapriv_wrapper->decrypt(p_content_str);
            saveFriendAesKey(from_id, msg_content);
            std::cout << "symmetric key received" << std::endl;
            break;
        case MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_MESSAGE_TEXT:
            try {
                msg_content = aes_wrapper.decrypt(p_content_str.c_str(), p_content_str.length());
                std::cout << msg_content << std::endl;
            } catch (std::exception& e) {
                std::cerr << "Exception: " << e.what() << "\n";
            }
            break;
        case MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_FILE:
            try {
                file_path += "%TMP%/file" + (++file_id);
                file_path += +".txt";
                my_file.set_path(file_path.c_str());
                msg_content = aes_wrapper.decrypt(p_content_str.c_str(), p_content_str.length());
                my_file.writeFile(msg_content);
                std::cout << file_path << std::endl;
            } catch (std::exception& e) {
                std::cerr << "Exception: " << e.what() << "\n";
            }
            break;
        default:
            std::cout << "bad MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE " << std::endl;
            break;
        }
        std::cout << "-----<EOM>-----" << std::endl;
        mem_ptr += sizeof(MUPRespGotMessagePayload) + msg_payload->message_size;
    }

    delete[] payloads;
    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::sendMsgClient(MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE type) {
    std::string readen_id;
    std::string content;
    std::string enc_content;
    FileRWIF my_file;
    
    std::cout << "please enter client id: ";
    std::getline(std::cin, readen_id);
    if (readen_id.length() != sizeof(clientId)) return OPERATION_STATUS_FAIL;

    switch (type)
    {
        case MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_GET_SYMETRIC_KEY:
            break;
        case MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_SYMETRIC_KEY:
            enc_content += encryptedAESKey(readen_id);
            if (enc_content.length() == 0) {
                return OPERATION_STATUS_FAIL;
            }
            break;
        case MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_MESSAGE_TEXT:
            std::cout << "please enter message: ";
            std::getline(std::cin, content);
            enc_content += encryptedMessage(readen_id, content);
            if (enc_content.length() == 0) {
                return OPERATION_STATUS_FAIL;
            }
            break;
        case MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_FILE:
            std::cout << "please enter file path: ";
            std::getline(std::cin, content);
            my_file.set_path(content.c_str());
            if (my_file.exists()) {
                content = my_file.getData();
                enc_content += encryptedMessage(readen_id, content);  
                if (enc_content.length() == 0) {
                    return OPERATION_STATUS_FAIL;
                }
            } else {
                std::cout << "file not found" << std::endl;
                return OPERATION_STATUS_FAIL;
            }
            break;
        default:
            std::cout << "bad MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE " << std::endl;
            break;
    }

    char *req_payload = new char[sizeof(MUPReqSendMessagePayload) + enc_content.length()];
    MUPReqSendMessagePayload* req_payload_ptr = (MUPReqSendMessagePayload*)req_payload;
    SET_MEM(req_payload_ptr->id.id, readen_id.c_str());
    req_payload_ptr->content_size = enc_content.length();
    req_payload_ptr->message_type = type;
    req_payload_ptr->message_content = &req_payload[sizeof(MUPReqSendMessagePayload)];
    memcpy(req_payload_ptr->message_content, enc_content.c_str(), req_payload_ptr->content_size);
    req.header.payload_size = sizeof(MUPReqSendMessagePayload) + enc_content.length();
    req.header.code = MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE;
    req.payload = req_payload;
    sendReq();
    delete [] req_payload;

    MUPRespMessageSentPayload resp_payload;
    RETURN_ON_FAIL(recvResp((char*)&resp_payload));
    std::string recev_id_str(resp_payload.id.id, sizeof(resp_payload.id.id));
    if (recev_id_str.compare(readen_id) != 0) {
        return OPERATION_STATUS_FAIL;
    }

    return OPERATION_STATUS_SUCCESS;
}

OPERATION_STATUS MessageUClient::execCMD(unsigned short int cmd) {
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
        getMsgsClient();
        break;
    case 50:
        sendMsgClient(MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_MESSAGE_TEXT);
        break;
    case 51:
        sendMsgClient(MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_GET_SYMETRIC_KEY);
        break;
    case 52:
        sendMsgClient(MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_SYMETRIC_KEY);
        break;
    case 53:
        sendMsgClient(MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_FILE);
         break;
    case 0:
        got_exit = true;
        break;
    default:
        std::cerr << "ERROR: Wrong cmd: " << cmd \
            << "," << std::endl << "please follow instuction!" << std::endl;
    }
    return OPERATION_STATUS_SUCCESS;
}