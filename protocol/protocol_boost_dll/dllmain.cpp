#include "pch.h"

#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/enum.hpp>
#include <boost/make_shared.hpp>

using namespace boost::python;


#include "..\MessageUProtocolReq.h"
#include "..\MessageUProtocolResp.h"

// static function declaration 

static list wrap_char_arr(char* arr, size_t size);
static str wrap_char_arr_as_str(char* arr, size_t size);
static list wrap_byte_arr(char* arr, size_t size);
static clientId get_zerro_id(clientId id);
static clientId id_add1(clientId id);

// wrapper classes
typedef enum {
	MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_NO_ERR,
	MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_HEADER,
	MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_PAYLOAD,
} MUP_REQ_MESSAGE_WRAPPER_ERR_CODE;


class MUPReqMessageWrapper {
public:
	MUPReqMessageWrapper(list py_buffer);
	MUP_REQ_MESSAGE_WRAPPER_ERR_CODE err;
	virtual ~MUPReqMessageWrapper() {};
	int code();
	MUPReqMessage msg;
	MUPReqRegistretionPayload registration_payload;
	MUPReqGetPublicKeyPayload get_public_key_payload;
	MUPReqSendMessagePayload send_message_payload;
};


//implementation

list wrap_char_arr(char* arr, size_t size) {
	list a;
	for (unsigned int i = 0; i < size; i++) { a.append(arr[i]); }
	return a;
}

str wrap_char_arr_as_str(char* arr, size_t size) {
	str a;
	for (unsigned int i = 0; i < size; i++) { a+=arr[i]; }
	return a;
}

list wrap_byte_arr(char* arr, size_t size) {
	list a;
	for (unsigned int i = 0; i < size; i++) { a.append(arr[i] & 0xff); }
	return a;
}

clientId get_zerro_id(clientId id) {
	clientId zerro_id;
	for (int i = 0; i < sizeof(zerro_id.id); i++) {
		zerro_id.id[i] = '0';
	}
	return zerro_id;
}

clientId id_add1(clientId id) {
	int i = 0;
	bool changed = false;
	while ((i < sizeof(id)) && !changed) {
		switch (id.id[i]) {
		case 'f':
			id.id[i] = '0';
			i++;
			break;
		case '9':
			id.id[i] = 'a';
			changed = true;
			break;
		default:
			id.id[i]++;
			changed = true;
		}
	}
	return id;
}


void wrap_message_type_write(MUPRespGotMessagePayload payload, MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE type) {
	payload.message_type = type;
}

MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE wrap_message_type_read(MUPRespGotMessagePayload payload) {
	return (MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE)payload.message_type;
}
MUPReqMessageWrapper::MUPReqMessageWrapper(list py_buffer) {
	printf("start MUPReqMessageWrapper\n");
	err = MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_NO_ERR;
	for (int i = 0; i < sizeof(msg.header); i++) {
		*(((unsigned char*)&msg.header) + i) = extract<unsigned char>(py_buffer[i]);
	}
	if ((uint32_t)len(py_buffer) != (uint32_t)(sizeof(msg.header) + msg.header.payload_size)) {
		printf("MUPReqMessageWrapper Error!!! given data is wrong size!! got  %d expected %d\n",
			(uint32_t)len(py_buffer), sizeof(msg.header) + msg.header.payload_size);
		fflush(stdout);
		err = MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_HEADER;
	}
	printf("header:\n");
	printf("\tcode: %u\n", msg.header.code);
	printf("\tid: %u\n", msg.header.id);
	printf("\tpayload_size: %u\n", msg.header.payload_size);
	printf("\tversion: %u\n", msg.header.version);
	printf("\terr: %d\n", err);
	fflush(stdout);
	if (MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_NO_ERR == err) {
		printf("payload:\n");
		switch (msg.header.code) {
		case MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION:
			for (unsigned int i = 0; i < msg.header.payload_size; i++) {
				*(((unsigned char*)&registration_payload) + i) = extract<unsigned char>(py_buffer[sizeof(msg.header) + i]);
			}
			msg.payload = (MUPPayload)&registration_payload;
			printf("\tname: %s\n", registration_payload.name.name);
			printf("\tpublic: %s\n", registration_payload.public_key.public_key);
			fflush(stdout);
			break;
		case MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST:
			break;
		case MUP_REQ_MESSAGE_COD_TYPE_GET_PUBLIC_KEY:
			for (unsigned int i = 0; i < msg.header.payload_size; i++) {
				*(((unsigned char*)&get_public_key_payload) + i) = extract<unsigned char>(py_buffer[sizeof(msg.header) + i]);
			}
			msg.payload = (MUPPayload)&get_public_key_payload;
			break;
		case MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE:
			for (unsigned int i = 0; i < msg.header.payload_size; i++) {
				*(((unsigned char*)&send_message_payload) + i) = extract<unsigned char>(py_buffer[sizeof(msg.header) + i]);
			}
			msg.payload = (MUPPayload)&send_message_payload;
			printf("\tid: %s\n", send_message_payload.id.id);
			printf("\tmeassage type: %d\n", send_message_payload.message_type);
			printf("\tcontent_size: %u\n", send_message_payload.content_size);
			if (send_message_payload.content_size) {
				printf("\tpublic: %s\n", send_message_payload.message_content);
			}
			
			fflush(stdout);
			break;
		default:
			printf("MUPReqMessageWrapper Error!!! bad payload type \n");
			fflush(stdout);
			err = MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_PAYLOAD;
		};
	}
}

int MUPReqMessageWrapper::code() {
	return (int)msg.header.code;
}


int identity_req_(MUP_REQ_MESSAGE_COD_CODE x) { return (int)x; };
int identity_resp_(MUP_RESP_MESSAGE_CODE x) { return (int)x; };
int identity_mgs_err_(MUP_REQ_MESSAGE_WRAPPER_ERR_CODE x) { return (int)x; };
int identity_mgs_(MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE x) { return (int)x; };

BOOST_PYTHON_MODULE(MessageUProtocol)
{
	class_<clientId, boost::shared_ptr<clientId>>("clientId", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				clientId x;
				ZERRO_MEM(x);
				return boost::make_shared<clientId>(x);
			});
			constructor(self);
		})
		.def("__init__", +[](object self, str id) {
			auto constructor = boost::python::make_constructor(+[](str id) {
				clientId x;
				ZERRO_MEM(x);
				char const* c_str = extract<char const*>(id);
				for (int i = 0; i < sizeof(x.id); i++) {
					x.id[i] = c_str[i];
				}
				return boost::make_shared<clientId>(x);
			});
			constructor(self, id);
		})
		.add_property("zerro_id", &get_zerro_id)
		.add_property("id", +[](clientId& self) {
			return wrap_char_arr_as_str(self.id, sizeof(self.id));
		})
		.def("add1", &id_add1)
		;

	class_<clientName, boost::shared_ptr<clientName>>("clientName", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				clientName x;
				ZERRO_MEM(x);
				return boost::make_shared<clientName>(x);
			});
			constructor(self);
		})
		.def("__init__", +[](object self, str id) {
			auto constructor = boost::python::make_constructor(+[](str id) {
				clientName x;
				ZERRO_MEM(x);
				char const* c_str = extract<char const*>(id);
				for (int i = 0; i < sizeof(x.name); i++) {
					x.name[i] = c_str[i];
				}
				return boost::make_shared<clientName>(x);
				});
			constructor(self, id);
			})
		.add_property("name", +[](clientName &self) {

			return wrap_char_arr_as_str(self.name, sizeof(self.name));
		})
		.add_property("bytes_arr", +[](clientName& self) {
			return wrap_byte_arr((char*)&self, sizeof(self));
		})
		;

	class_<clientPublicKey, boost::shared_ptr<clientPublicKey>>("clientPublicKey", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				clientPublicKey x;
				ZERRO_MEM(x);
				return boost::make_shared<clientPublicKey>(x);
			});
			constructor(self);
		})
		.def("__init__", +[](object self, list public_key) {
			auto constructor = boost::python::make_constructor(+[](list public_key) {
				clientPublicKey x;
				ZERRO_MEM(x);
				unsigned char lch, bch;
				for (int i = 0; i < sizeof(x.public_key); i++) {
					bch= (unsigned char)extract<unsigned int>(public_key[i*2]);
					lch = (unsigned char)extract<unsigned int>(public_key[i*2+1]);
					if ('9' >= lch) lch = lch - '0';
					else if ('a' <= lch) lch = lch - ('a' - 0xa);

					if ('9' >= bch) bch = bch - '0';
					else if ('a' <= bch) bch = bch - ('a' - 0xa);

					printf("\public_key %d : 0x%x\n", i, (bch << 4) | lch);
					fflush(stdout);
					x.public_key[i] = (bch << 4) | lch;
				}
				return boost::make_shared<clientPublicKey>(x);
			});
			constructor(self, public_key);
		})
		.add_property("public_key", +[](clientPublicKey& self) {
			return wrap_byte_arr(self.public_key, sizeof(self.public_key));
		})
		;
	enum_<MUP_REQ_MESSAGE_COD_CODE>("MUP_REQ_MESSAGE_COD_CODE")
		.value("MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION", MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION)
		.value("MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST", MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST)
		.value("MUP_REQ_MESSAGE_COD_TYPE_GET_PUBLIC_KEY", MUP_REQ_MESSAGE_COD_TYPE_GET_PUBLIC_KEY)
		.value("MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE", MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE)
		.value("MUP_REQ_MESSAGE_COD_TYPE_GET_MESSAGES", MUP_REQ_MESSAGE_COD_TYPE_GET_MESSAGES)
		.export_values()
		;
	def("identity", identity_req_);

	class_<MUPReqHeader, boost::shared_ptr<MUPReqHeader>>("MUPReqHeader", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPReqHeader x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPReqHeader>(x);
			});
			constructor(self);
		})
		.def_readonly("id", &MUPReqHeader::id)
		.def_readonly("version", &MUPReqHeader::version)
		/*.def_readonly("code", +[](const MUPReqHeader& a) {return (int)(a.code); })*/
		.def_readonly("payload_size", &MUPReqHeader::payload_size)
		;

	class_<MUPReqRegistretionPayload, boost::shared_ptr<MUPReqRegistretionPayload>>("MUPReqRegistretionPayload", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPReqRegistretionPayload x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPReqRegistretionPayload>(x);
			});
			constructor(self);
		})
		.def_readonly("name", &MUPReqRegistretionPayload::name)
		.def_readonly("public_key", &MUPReqRegistretionPayload::public_key)
		;

	class_<MUPReqGetPublicKeyPayload, boost::shared_ptr<MUPReqGetPublicKeyPayload>>("MUPReqGetPublicKeyPayload", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPReqGetPublicKeyPayload x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPReqGetPublicKeyPayload>(x);
				});
			constructor(self);
		})
		.def_readonly("id", &MUPReqGetPublicKeyPayload::id)
		.add_property("bytes_arr", +[](MUPReqGetPublicKeyPayload& self) {
			return wrap_byte_arr((char*)&self, sizeof(self));
		})
		;


	class_<MUPReqSendMessagePayload, boost::shared_ptr<MUPReqSendMessagePayload>>("MUPReqSendMessagePayload", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPReqSendMessagePayload x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPReqSendMessagePayload>(x);
			});
			constructor(self);
		})
		.add_property("id", &MUPReqSendMessagePayload::id)
		.add_property("message_type", +[](MUPReqSendMessagePayload &self) {
			return self.message_type;
		})
		.add_property("content_size", &MUPReqSendMessagePayload::content_size)
		.add_property("message_content", +[](MUPReqSendMessagePayload &self) {
			return  wrap_byte_arr(self.message_content, self.content_size);
		})
		;

	class_<MUPReqMessage, boost::shared_ptr<MUPReqMessage>>("MUPReqMessage", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPReqMessage x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPReqMessage>(x);
			});
			constructor(self);
		})
		.add_property("header", &MUPReqMessage::header)
		.add_property("bytes_arr", +[](MUPReqMessage &self) {
			return wrap_byte_arr((char*)&self, sizeof(self));
		})
		;

	enum_<MUP_RESP_MESSAGE_CODE>("MUP_RESP_MESSAGE_CODE")
		.value("MUP_RESP_MESSAGE_CODE_REGISTRETION_SUCCESSED", MUP_RESP_MESSAGE_CODE_REGISTRETION_SUCCESSED)
		.value("MUP_RESP_MESSAGE_CODE_CLIENTS_LIST", MUP_RESP_MESSAGE_CODE_CLIENTS_LIST)
		.value("MUP_RESP_MESSAGE_CODE_GET_PUBLIC_KEY", MUP_RESP_MESSAGE_CODE_GET_PUBLIC_KEY)
		.value("MUP_RESP_MESSAGE_CODE_MESSAGE_SENT", MUP_RESP_MESSAGE_CODE_MESSAGE_SENT)
		.value("MUP_RESP_MESSAGE_COD_TYPE_GET_MESSAGES", MUP_RESP_MESSAGE_COD_TYPE_GET_MESSAGES)
		.export_values()
		;
	def("identity", identity_resp_);

	enum_<MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE>("MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE")
		.value("MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_GET_SYMETRIC_KEY", MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_GET_SYMETRIC_KEY)
		.value("MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_SYMETRIC_KEY", MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_SYMETRIC_KEY)
		.value("MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_MESSAGE_TEXT", MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_MESSAGE_TEXT)
		.value("MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_FILE", MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE_SEND_FILE)
		.export_values()
		;
	def("identity", identity_mgs_);

	class_<MUPRespHeader>("MUPRespHeader", no_init)
		.def("__init__", +[](object self) {
		auto constructor = boost::python::make_constructor(+[]() {
			MUPRespHeader x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPRespHeader>(x);
			});
			constructor(self);
		})
		.add_property("version", &MUPRespHeader::version, &MUPRespHeader::version)
		.add_property("code", &MUPRespHeader::code, &MUPRespHeader::code)
		.add_property("payload_size", &MUPRespHeader::payload_size, &MUPRespHeader::payload_size)
		.add_property("bytes_arr", +[](MUPRespHeader& self) {
			return wrap_byte_arr((char*)&self, sizeof(self));
		})
		;

	class_<MUPRespRegistretionSuccessedPayload, boost::shared_ptr<MUPRespRegistretionSuccessedPayload>>
		("MUPRespRegistretionSuccessedPayload", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPRespRegistretionSuccessedPayload x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPRespRegistretionSuccessedPayload>(x); 
				});
			constructor(self);
		})
		.def("__len__", +[](object self) {
			return sizeof(MUPRespRegistretionSuccessedPayload);
		})
		.add_property("id", &MUPRespRegistretionSuccessedPayload::id, &MUPRespRegistretionSuccessedPayload::id)
		.add_property("bytes_arr", +[](MUPRespRegistretionSuccessedPayload& self) {
			return wrap_byte_arr((char*)&self, sizeof(self));
		})
		;

	class_<MUPReqResClientStruct, boost::shared_ptr<MUPReqResClientStruct>>("MUPReqResClientStruct", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPReqResClientStruct x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPReqResClientStruct>(x);
			});
			constructor(self);
		})
		.add_property("id", &MUPReqResClientStruct::id, &MUPReqResClientStruct::id)
		.add_property("name", &MUPReqResClientStruct::name, &MUPReqResClientStruct::name)
		.add_property("bytes_arr", +[](MUPReqResClientStruct& self) {
			return wrap_byte_arr((char*)&self, sizeof(self));
		})
		;

	class_<MUPRespGetPublicKeyPayload, boost::shared_ptr<MUPRespGetPublicKeyPayload>>("MUPRespGetPublicKeyPayload", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPRespGetPublicKeyPayload x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPRespGetPublicKeyPayload>(x);
			});
			constructor(self);
		})
		.add_property("id", &MUPRespGetPublicKeyPayload::id, &MUPRespGetPublicKeyPayload::id)
		.add_property("publicKey", &MUPRespGetPublicKeyPayload::publicKey, &MUPRespGetPublicKeyPayload::publicKey)
		.add_property("bytes_arr", +[](MUPRespGetPublicKeyPayload& self) {
			return wrap_byte_arr((char*)&self, sizeof(self));
		})
		;

	class_<MUPRespMessageSentPayload, boost::shared_ptr<MUPRespMessageSentPayload>>("MUPRespMessageSentPayload", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPRespMessageSentPayload x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPRespMessageSentPayload>(x);
			});
			constructor(self);
		})
		.add_property("id", &MUPRespMessageSentPayload::id, &MUPRespMessageSentPayload::id)
		.add_property("message_id", &MUPRespMessageSentPayload::message_id, &MUPRespMessageSentPayload::message_id)
		.add_property("bytes_arr", +[](MUPRespMessageSentPayload& self) {
			return wrap_byte_arr((char*)&self, sizeof(self));
		})
		;

	class_<MUPRespGotMessagePayload, boost::shared_ptr<MUPRespGotMessagePayload>>("MUPRespGotMessagePayload", no_init)
		.def("__init__", +[](object self) {
			auto constructor = boost::python::make_constructor(+[]() {
				MUPRespGotMessagePayload x;
				ZERRO_MEM(x);
				return boost::make_shared<MUPRespGotMessagePayload>(x);
			});
			constructor(self);
		})
		.add_property("id", &MUPRespGotMessagePayload::id, &MUPRespGotMessagePayload::id)
		.add_property("message_id", &MUPRespGotMessagePayload::message_id, &MUPRespGotMessagePayload::message_id)
		.add_property("message_type", &wrap_message_type_read, &wrap_message_type_write)
		.add_property("message_size", &MUPRespGotMessagePayload::message_size, &MUPRespGotMessagePayload::message_size)
		.add_property("content", &MUPRespGotMessagePayload::content, &MUPRespGotMessagePayload::content)
		;

		
	enum_<MUP_REQ_MESSAGE_WRAPPER_ERR_CODE>("MUP_REQ_MESSAGE_WRAPPER_ERR_CODE")
		.value("MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_HEADER", MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_HEADER)
		.value("MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_PAYLOAD", MUP_REQ_MESSAGE_WRAPPER_ERR_CODE_PAYLOAD)
		.export_values()
		;
		def("identity", identity_req_);

	class_<MUPReqMessageWrapper, boost::shared_ptr<MUPReqMessageWrapper>>
		("MUPReqMessageWrapper", no_init)
		.def("__init__", +[](object self, list bytes) {
			auto constructor = boost::python::make_constructor(+[](list bytes) {
				MUPReqMessageWrapper x(bytes);
				return boost::make_shared<MUPReqMessageWrapper>(x);
			});
			constructor(self, bytes);
		})

		.add_property("msg", &MUPReqMessageWrapper::msg)
		.add_property("registration_payload", &MUPReqMessageWrapper::registration_payload)
		.add_property("get_public_key_payload", &MUPReqMessageWrapper::get_public_key_payload)
		.add_property("send_message_payload", &MUPReqMessageWrapper::send_message_payload)
		.add_property("code", &MUPReqMessageWrapper::code)
		.add_property("err", &MUPReqMessageWrapper::err)
		;
};

#if PY_MAJOR_VERSION >= 3
#   define INIT_MODULE PyInit_MessageUProtocol
extern "C" PyObject * INIT_MODULE();
#else
#   define INIT_MODULE initmymodule
extern "C" void INIT_MODULE();
#endif

int main(int argc, char* argv[])
{
	error_already_set e;
	try {
		PyImport_AppendInittab("MessageUProtocol", INIT_MODULE);
		Py_Initialize();
	}
	catch (error_already_set& e) {
		PyErr_PrintEx(0);
		return 1;
	}
	return 0;
}