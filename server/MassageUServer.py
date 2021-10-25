import enum
import datetime

from server.utils import SQL
from server.Server import Server

import protocol.protocol_boost_dll.Debug.MessageUProtocol as MessageUProtocol

b = list(b'01000000000000000000000000000000\x01\x00\x00\x00\xeb\x03\x00\x00,\x00\x00\x0001000000000000000000000000000000\x01\xcd\xcd\xcd\x00\x00\x00\x00\xcd\xcd\xcd\xcd')
m = MessageUProtocol.MUPReqMessageWrapper(b)
#
# print(m.send_message_payload.id.id)
# print(m.send_message_payload.message_type)
# print("".join([f"{x:0{2}x}" for x in m.send_message_payload.message_content]))
# print("--------------------------------")
# # print("id = ", "".join(m.msg.header.id.id))
# print("version = ", m.msg.header.version)
# print("code = ", m.code)
# print("payload_size = ", m.msg.header.payload_size)
# if MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(m.code):
#     print("-----------registration_payload-------------")
#     print("name = ", m.registration_payload.name.name)
#     print("public key = ", m.registration_payload.public_key.public_key)
# p = MessageUProtocol.MUPRespRegistretionSuccessedPayload()
# d = MessageUProtocol.clientId().zerro_id
# d = d.add1()
# p.id = d
# print(p.id.id)
# print(len(p))
# print(p.bytes_arr)
sql_db = SQL()
# payload = bytes()
# sql_list = sql_db.get_clients()
# print(sql_list)
# for client_id in sql_list.keys():
#     c = MessageUProtocol.MUPReqResClientStruct()
#     c.id = MessageUProtocol.clientId(client_id)
#     c.name = MessageUProtocol.clientName(sql_list[client_id]['name'])
#     payload = payload + bytes(c.bytes_arr)
# print(payload)

# sql_list = sql_db.get_client('b0000000000000000000000000000000')

# l = list(bytes(sql_list['b0000000000000000000000000000000']['public_key'], 'utf-8'))
# l = [i<< for i, _ in enumerate(l[::2])]
# print ([])

# pub = list(bytes(sql_list['b0000000000000000000000000000000']['public_key'], 'utf-8'))
# z_pref =list(bytes([ord('0')] * (160*2 - len(pub))))
# # pub = pub
# print()
# MessageUProtocol.clientPublicKey(z_pref + pub)
#
message_id = sql_db.add_message(m.msg.header.id.id,
                                     m.send_message_payload.id.id,
                                     m.send_message_payload.message_type,
                                     "".join([f"{x:0{2}x}" for x in m.send_message_payload.message_content]))



class REQ_STATUS(enum.Enum):
    recv = 0
    process = 1
    answer = 2
    send = 3
    bad = 4


class CONNECTION_STATUS(enum.Enum):
    active = 0
    # holding = 2
    end = 3


class MessageUClient:
    def __init__(self, client_id=None, connection=None, connection_status=CONNECTION_STATUS.active, request=None,
                 request_status=REQ_STATUS.recv, sql_db=None):
        self.client_id = client_id
        self.connection = connection
        self.connection_status = connection_status
        self.request = request
        self.request_status = request_status
        self.response = MessageUProtocol.MUPRespHeader()
        self.sql_db = sql_db
        self.payload = None
        self.cliens_list = None
        self.last_seen = datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')

    def serv(self):
        while self.connection_status == CONNECTION_STATUS.active:
            if self.request_status == REQ_STATUS.recv:
                self.recv()
            elif self.request_status == REQ_STATUS.process:
                self.proccess()
            elif self.request_status == REQ_STATUS.answer:
                self.answer()
            elif self.request_status == REQ_STATUS.send:
                self.send()
            elif self.request_status == REQ_STATUS.bad:
                self.connection.send(b"ERROR!!!!" + Server.END_OF_PACKET)
                self.connection.close()
                self.connection_status = CONNECTION_STATUS.end

    def recv(self):
        try:
            data = self.connection.recv(Server.BUFFER_SIZE)
            print(data)
            if len(data) <= 0:
                print("client disconnected")
                self.connection_status = CONNECTION_STATUS.end
                return 1
            self.request = MessageUProtocol.MUPReqMessageWrapper(list(data))
            if self.request.err:
                print("MessageUClient, got Request with err byte stream:", list(data))
                print("MessageUServer, err:", self.request.err)
                self.request_status = REQ_STATUS.bad
                return 1
            self.last_seen = datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')
            if MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
                print("Error Client already connected")
                self.request_status = REQ_STATUS.bad
            elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
                self.request_status = REQ_STATUS.process
            elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_PUBLIC_KEY == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
                self.request_status = REQ_STATUS.process
            elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
                self.request_status = REQ_STATUS.process
            else:
                self.request_status = REQ_STATUS.bad
        except MemoryError:
            self.connection_status = CONNECTION_STATUS.end
            print("client disconnected")
        except ConnectionResetError:
            self.connection_status = CONNECTION_STATUS.end
            print("client disconnected")
        except ConnectionAbortedError:
            self.connection_status = CONNECTION_STATUS.end
            print("client disconnected")

    def proccess(self):
        if MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            self.sql_db.add_client(self.client_id.id,
                                   self.request.registration_payload.name.name.strip('\0'),
                                   "".join([f"{x:0{2}x}" for x in self.request.registration_payload.public_key.public_key]),
                                   self.last_seen)
            self.request_status = REQ_STATUS.answer
        elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            self.request_status = REQ_STATUS.answer
        elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_PUBLIC_KEY == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            self.request_status = REQ_STATUS.answer
        elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            self.request_status = REQ_STATUS.answer
        else:
            self.request_status = REQ_STATUS.bad

    def answer(self):
        if MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            self.response.code = MessageUProtocol.MUP_RESP_MESSAGE_CODE_REGISTRETION_SUCCESSED
            self.response.version = self.request.msg.header.version
            self.payload = MessageUProtocol.MUPRespRegistretionSuccessedPayload()
            self.payload.id = self.client_id
            self.request_status = REQ_STATUS.send
        elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            self.response.code = MessageUProtocol.MUP_RESP_MESSAGE_CODE_CLIENTS_LIST
            self.payload = bytes()
            sql_list = self.sql_db.get_clients()
            clients = list(set(sql_list.keys()) - set([self.client_id.id]))
            for client_id in clients:
                c = MessageUProtocol.MUPReqResClientStruct()
                c.id = MessageUProtocol.clientId(client_id)
                c.name = MessageUProtocol.clientName(sql_list[client_id]['name'])
                self.payload = self.payload + bytes(c.bytes_arr)
            self.request_status = REQ_STATUS.send
        elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_PUBLIC_KEY == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            self.response.code = MessageUProtocol.MUP_RESP_MESSAGE_CODE_GET_PUBLIC_KEY
            self.response.version = self.request.msg.header.version
            try:
                sql_list = self.sql_db.get_client(self.request.get_public_key_payload.id.id)
                print("sql_list", sql_list)
                if len(sql_list):
                    self.payload = MessageUProtocol.MUPRespGetPublicKeyPayload()
                    self.payload.id = MessageUProtocol.clientId(self.request.get_public_key_payload.id.id)
                    pub = list(bytes(sql_list[self.request.get_public_key_payload.id.id]['public_key'], 'utf-8'))
                    self.payload.publicKey = MessageUProtocol.clientPublicKey(pub)
                    self.request_status = REQ_STATUS.send
                else:
                    self.request_status = REQ_STATUS.bad
            except UnicodeDecodeError:
                self.request_status = REQ_STATUS.bad
        elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            self.payload = MessageUProtocol.MUPRespMessageSentPayload()
            self.response.code = MessageUProtocol.MUP_RESP_MESSAGE_CODE_MESSAGE_SENT
            self.response.version = 2
            message_id = self.sql_db.add_message(self.request.msg.header.id.id,
                                                 self.request.send_message_payload.id.id,
                                                 self.request.send_message_payload.message_type,
                                                 "".join([f"{x:0{2}x}" for x in self.request.send_message_payload.message_content]))
            self.payload.id = self.request.send_message_payload.id
            self.payload.message_id = message_id
            if message_id is None:
                self.request_status = REQ_STATUS.bad
            else:
                self.request_status = REQ_STATUS.send
        else:
            self.request_status = REQ_STATUS.bad

    def send(self):
        if not type(self.payload) == type(bytes()):
            self.payload = bytes(self.payload.bytes_arr)
        self.response.payload_size = len(self.payload)
        print("send :", bytes(self.response.bytes_arr) + self.payload + Server.END_OF_PACKET)
        self.connection.send(bytes(self.response.bytes_arr) + self.payload + Server.END_OF_PACKET)
        self.request_status = REQ_STATUS.recv


class MessageUServer(Server):
    def __str__(self):
        return 'MessageU' + super(MessageUServer, self).__str__()

    def __init__(self, port: int = 8080):
        super(MessageUServer, self).__init__(port)
        self.sql_db = SQL()
        self.last_client_id = self.update_last_client_id()

    def update_last_client_id(self):
        last_id = self.sql_db.get_last_id('clients')
        if last_id == None:
            return MessageUProtocol.clientId().zerro_id
        else:
            return MessageUProtocol.clientId(last_id)

    def inc_last_client_id(self):
        self.last_client_id = self.last_client_id.add1()

    def recv(self, con):
        print("---------------------------------------------------------------")
        print("TODO: Free long last seen")
        data = super(MessageUServer, self).recv(con)
        print(data)
        if len(data) <= 0:
            print("client disconnected")
            return ''
        try:
            mup_req = MessageUProtocol.MUPReqMessageWrapper(list(data))
            if mup_req.err:
                print("MessageUServer, got Request with err byte stream:", data)
                print("MessageUServer, err:", mup_req.err)
                con.send(b"ERROR!!!!" + Server.END_OF_PACKET)
                con.close()
                return ''
            if MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(mup_req.code):
                print(list(mup_req.registration_payload.name.bytes_arr))
                if self.sql_db.name_exists(mup_req.registration_payload.name.name.strip('\0')):
                    con.send(b"ERROR!!!!" + Server.END_OF_PACKET)
                    con.close()
                else:
                    self.inc_last_client_id()
                    client = MessageUClient(client_id=self.last_client_id, connection=con,
                                            connection_status=CONNECTION_STATUS.active, request=mup_req,
                                            request_status=REQ_STATUS.process, sql_db=self.sql_db)
                    self.clients[client.client_id] = client
                    self.clients[client.client_id].request_status = REQ_STATUS.process
                    self.clients[client.client_id].serv()
            else:
                if self.sql_db.id_exists(mup_req.msg.header.id.id):
                    client = MessageUClient(client_id=MessageUProtocol.clientId(mup_req.msg.header.id.id), connection=con,
                                            connection_status=CONNECTION_STATUS.active, request=mup_req,
                                            request_status=REQ_STATUS.process, sql_db=self.sql_db)
                    self.clients[client.client_id] = client
                    self.clients[client.client_id].request_status = REQ_STATUS.process
                    self.clients[client.client_id].serv()
                else:
                    con.send(b"ERROR!!!!" + Server.END_OF_PACKET)
                    con.close()
        except MemoryError:
            print("client disconnected")

