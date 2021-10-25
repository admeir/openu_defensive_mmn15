#pragma once
#include "../client/client/Utils/Utils.h"

typedef enum {
	MUP_RESP_MESSAGE_CODE_REGISTRETION_SUCCESSED = 2000,
	MUP_RESP_MESSAGE_CODE_CLIENTS_LIST,
	MUP_RESP_MESSAGE_CODE_GET_PUBLIC_KEY,
	MUP_RESP_MESSAGE_CODE_MESSAGE_SENT,
	MUP_RESP_MESSAGE_COD_TYPE_GET_MESSAGES,
	MUP_RESP_MESSAGE_CODE_ERR,
}MUP_RESP_MESSAGE_CODE;

typedef struct _MUPRespHeader {
	uint8_t version;
	uint16_t code;
	uint32_t payload_size;
} MUPRespHeader;

typedef struct _MUPRespRegistretionSuccessedPayload{
	clientId id;
} MUPRespRegistretionSuccessedPayload;

typedef struct _MUPReqResClient {
	clientId id;
	clientName name;
} MUPReqResClientStruct;

typedef struct _MUPReqResClientsPayload {
	MUPReqResClientStruct *clients;
} MUPReqResClientsPayload;

typedef struct _MUPRespGetPublicKeyPayload {
	clientId id;
	clientPublicKey publicKey;
} MUPRespGetPublicKeyPayload;

typedef struct _MUPRespMessageSentPayload {
	clientId id;
	uint32_t message_id;
} MUPRespMessageSentPayload;


typedef struct _MUPRespGotMessagePayload {
	clientId id;
	uint32_t message_id;
	MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE message_type : MUP_MESSAGE_PAYLOAD_TYPE_SIZE;
	uint32_t message_size;
	char* content;
} MUPRespGotMessagePayload;

typedef struct _MUPRespMessage {
	MUPRespHeader header;
	MUPPayload payload;
} MUPRespMessage;