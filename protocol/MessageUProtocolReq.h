#pragma once

#include "../client/client/Utils/Utils.h"

typedef enum {
	MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION = 1000,
	MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST,
	MUP_REQ_MESSAGE_COD_TYPE_GET_PUBLIC_KEY,
	MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE,
}MUP_REQ_MESSAGE_COD_CODE;

typedef struct _MUPReqHeader {
	clientId id;
	uint8_t version;
	MUP_REQ_MESSAGE_COD_CODE code: MUP_MESSAGE_CODE_SIZE;
	uint32_t payload_size;
} MUPReqHeader;

typedef struct _MUPReqRegistretionPayload {
	clientName name;
	clientPublicKey public_key;
} MUPReqRegistretionPayload;

typedef struct _MUPReqGetPublicGetKeyPayload {
	clientId id;
} MUPReqGetPublicKeyPayload;


typedef struct _MUPReqSendMessagePayload {
	clientId id;
	MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE message_type : MUP_MESSAGE_PAYLOAD_TYPE_SIZE;
	uint32_t content_size;
	char *message_content;
} MUPReqSendMessagePayload;

typedef struct _MUPReqMessage{
	MUPReqHeader header;
	MUPPayload payload;
} MUPReqMessage;