
#include <cstdio>
#include "MessageUProtocolReq.h"
#include "MessageUProtocolResp.h"
#define BOOST_PYTHON_STATIC_LIB 
#define PY_MAJOR_VERSION 3
#define PY_MINOR_VERSION 8

#include <boost/python.hpp>
using namespace boost::python;


BOOST_PYTHON_MODULE(MessageUProtocolResp)
{
	enum_<MUP_REQ_MESSAGE_COD_CODE>("MUP_REQ_MESSAGE_COD_CODE")
		.value("MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION", MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION)
		.value("MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST", MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST)
		.value("MUP_REQ_MESSAGE_COD_TYPE_GET_KEY_PAYLOA", MUP_REQ_MESSAGE_COD_TYPE_GET_KEY_PAYLOA)
		.value("MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE", MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE)
		;

	class_<MUPReqHeader>("MUPReqHeader")
		.def_readonly("id", &MUPReqHeader::id)
		.def_readonly("version", &MUPReqHeader::version)
		.def_readonly("code", [](MUPReqHeader& a) {
				return (MUP_REQ_MESSAGE_COD_CODE)(a.code & 0xff);
			} )
		.def_readonly("payload_size", &MUPReqHeader::payload_size)
		;

	class_<MUPReqRegistretionPayload>("MUPReqRegistretionPayload")
		.def_readonly("name", &MUPReqRegistretionPayload::name)
		.def_readonly("public_key", &MUPReqRegistretionPayload::public_key)
		;

	class_<MUPReqGetPublicKeyPayload>("MUPReqGetPublicKeyPayload")
		.def_readonly("id", &MUPReqGetPublicKeyPayload::id)
		;


	class_<MUPReqSendMessagePayload>("MUPReqGetPublicKeyPayload")
		.def_readonly("id", &MUPReqSendMessagePayload::id)
		.def_readonly("message_type", [](MUPReqSendMessagePayload& a) {
				return (MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE)(a.message_type & 0xf);
			})
		.def_readonly("content_size", &MUPReqSendMessagePayload::content_size)
		.def_readonly("message_content", [](MUPReqSendMessagePayload& a) {
				std::string message_content(a.message_content, a.content_size);
				return message_content;
			})
		;

		class_<MUPReqMessage>("MUPReqMessage")
			.def_readonly("header", &MUPReqMessage::header)
			.def_readonly("payload", [](MUPReqMessage& a) {
				std::string payload(a.payload, a.header.payload_size);
				return payload;
			})
		;
}

