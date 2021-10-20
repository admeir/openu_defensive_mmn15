#pragma once

#include <boost/asio.hpp>
#include <stdint.h>

#define SERVER_ERR 9000

#define MUP_MESSAGE_CODE_SIZE 16
#define ID_SIZE 16 * 2 // each cuple ascii chars demonstrate real 8 bits char
#define MAX_NAEM_SIZE 255
#define KEY_SIZE 160
#define PUBLIC_KEY_SIZE KEY_SIZE
#define PRIVATE_KEY_SIZE KEY_SIZE

typedef char* MUPPayload;

#define MUP_MESSAGE_PAYLOAD_TYPE_SIZE 8
typedef enum {
	MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_GET_SYMETRIC_KEY,
	MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_SYMETRIC_KEY,
	MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_MESSAGE_TEXT,
	MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_FILE,
}MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE;

typedef struct { 
	char id[ID_SIZE];
} clientId;

typedef struct {
	char name[MAX_NAEM_SIZE];
} clientName;

typedef struct {
	char public_key[PUBLIC_KEY_SIZE];
} clientPublicKey;

typedef enum {
	OPERATION_STATUS_SUCCESS,
	OPERATION_STATUS_FAIL,
} OPERATION_STATUS;

#define EXIT_ON_FAIL(foo) do { \
	int status = foo; \
	if (OPERATION_STATUS_SUCCESS != status) { \
		exit(status); \
	} \
} while(0)

#define RETURN_ON_FAIL(foo) do { \
	OPERATION_STATUS status = foo; \
	if (OPERATION_STATUS_SUCCESS != status) { \
		return status; \
	} \
} while(0)

#define RETURN_ON_FAIL_WITH_MSG(foo, msg) do { \
	OPERATION_STATUS status = foo; \
	if (OPERATION_STATUS_SUCCESS != status) { \
		std::cerr << msg << std::endl; \
		return status; \
	} \
} while(0)

#define ZERRO_MEM(var) do { \
	memset(&var, 0x00, sizeof(var)); \
} while(0)

#define SET_MEM(var, vlaue) do { \
	memcpy(&var, vlaue, sizeof(var)); \
} while(0)

typedef std::vector<std::string> splitedString;


bool isNumber(const std::string& s);
bool isValidIpAddress(std::string &ipAddress);
bool string_is_hex(const std::string & input);
void hexify(const unsigned char* buffer, unsigned int length);
OPERATION_STATUS validate(std::string& text, std::string& delimiter);
splitedString split(std::string& text, std::string& delimiter);
std::string join(splitedString str_list, std::string& delimiter);