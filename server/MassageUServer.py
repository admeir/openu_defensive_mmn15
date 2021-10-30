import enum
import datetime
import threading

from server.utils import SQL
from server.Server import Server

# if you want to run in debug mode uncomment next raw and comment the release raw
# import protocol.protocol_boost_dll.Debug.MessageUProtocol as MessageUProtocol
import protocol.protocol_boost_dll.Release.MessageUProtocol as MessageUProtocol

class REQ_STATUS(enum.Enum):
    recv = 0
    answer = 2
    send = 3
    bad = 4


class CONNECTION_STATUS(enum.Enum):
    active = 0
    block = 2
    end = 3


class MessageUClient:
    def close(self):
        self.connection.close()
        self.connection_status = CONNECTION_STATUS.end

    def __init__(self, client_id=None, connection=None, connection_status=CONNECTION_STATUS.active, request=None,
                 request_status=REQ_STATUS.recv):
        self.client_id = client_id
        self.connection = connection
        self.connection_status = connection_status
        self.request = request
        self.request_status = request_status
        self.response = MessageUProtocol.MUPRespHeader()
        self.response.version = 2
        self.sql_db = None
        self.payload = None
        self.cliens_list = None
        self.last_seen = datetime.datetime.utcnow()

    def serv(self):
        self.sql_db = SQL()
        while self.connection_status == CONNECTION_STATUS.active:
            if self.request_status == REQ_STATUS.recv:
                self.recv()
            elif self.request_status == REQ_STATUS.answer:
                self.answer()
            elif self.request_status == REQ_STATUS.send:
                self.send()
            elif self.request_status == REQ_STATUS.bad:
                self.response.code = MessageUProtocol.MUP_RESP_MESSAGE_CODE_ERR
                self.payload = bytes([])
                self.request_status = REQ_STATUS.send
        self.close()

    def check(self):
        diff_in_seconds = (datetime.datetime.utcnow() - self.last_seen).total_seconds()
        if diff_in_seconds < 5:
            print("Error, we are under attack!!!!")
            self.connection_status = CONNECTION_STATUS.block

    def recv(self):
        try:
            data = self.connection.recv(Server.BUFFER_SIZE)
            while len(data) == Server.BUFFER_SIZE:
                data += self.connection.recv(Server.BUFFER_SIZE)
            if len(data) <= 0:
                print("client disconnected")
                self.connection_status = CONNECTION_STATUS.end
                return 1
            try:
                self.request = MessageUProtocol.MUPReqMessageWrapper(list(data))
            except:
                print('Error!!! Server Can\'t sent message')
                self.request_status = REQ_STATUS.bad
            if self.request.err:
                print("MessageUClient, got Request with err byte stream:", list(data))
                print("MessageUServer, err:", self.request.err)
                self.request_status = REQ_STATUS.bad
                return 1

            self.last_seen = datetime.datetime.utcnow()
            if MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code) in \
                    [MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_CLIENT_LIST,
                     MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_PUBLIC_KEY,
                     MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE,
                     MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_MESSAGES]:
                self.request_status = REQ_STATUS.answer
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

    def add_client(self, sql_db):
        sql_db.add_client(self.client_id.id,
                           self.request.registration_payload.name.name.strip('\0'),
                           "".join([f"{x:0{2}x}" for x in self.request.registration_payload.public_key.public_key]),
                           self.last_seen.strftime('%Y-%m-%d %H:%M:%S'))
        self.request_status = REQ_STATUS.answer

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
        elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_GET_MESSAGES == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            sql_msgs = self.sql_db.get_messages(self.request.msg.header.id.id)
            self.response.code = MessageUProtocol.MUP_RESP_MESSAGE_COD_TYPE_GET_MESSAGES
            self.payload = bytes()
            for msg in sql_msgs:
                m = MessageUProtocol.MUPRespGotMessagePayload()
                m.id = MessageUProtocol.clientId(msg['FromClient'])
                m.message_id = msg['ID']
                m.message_type = MessageUProtocol.MUP_REQ_SEND_MESSAGE_PAYLOAD_TYPE(int(msg['Type']))
                content = msg['Content']
                content = bytes([int(''.join([content[2 * i], content[2 * i + 1]]), 16) for i, x in enumerate(content[::2])])
                m.message_size = len(content)
                self.payload = self.payload + bytes(m.bytes_arr) + content
                self.sql_db.delete_messages(msg['ID'])
            self.request_status = REQ_STATUS.send
        elif MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_SEND_MESSAGE == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(self.request.code):
            self.payload = MessageUProtocol.MUPRespMessageSentPayload()
            self.response.code = MessageUProtocol.MUP_RESP_MESSAGE_CODE_MESSAGE_SENT
            message_id = self.sql_db.add_message(self.request.msg.header.id.id,
                                                 self.request.send_message_payload.id.id,
                                                 self.request.send_message_payload.message_type,
                                                 "".join([f"{x:0{2}x}" for x in self.request.msg_content_buffer]))
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
        self.connection.send(bytes(self.response.bytes_arr) + self.payload + Server.END_OF_PACKET)
        self.request_status = REQ_STATUS.recv


class MessageUServer(Server):
    def __str__(self):
        return 'MessageU' + super(MessageUServer, self).__str__()

    def __init__(self, port: int = 8080):
        super(MessageUServer, self).__init__(port)
        self.sql_db = SQL()
        self.err_response = MessageUProtocol.MUPRespHeader()
        self.err_response.version = 2
        self.err_response.code = MessageUProtocol.MUP_RESP_MESSAGE_CODE_ERR
        self.err_response.payload_size = 0
        self.last_client_id = self.update_last_client_id()

    def update_last_client_id(self):
        last_id = self.sql_db.get_last_id('clients')
        if last_id == None:
            return MessageUProtocol.clientId().zerro_id
        else:
            return MessageUProtocol.clientId(last_id)

    def inc_last_client_id(self):
        self.last_client_id = self.last_client_id.add1()

    def disconnect_long_last_seen(self):
        for client_id in self.clients.keys():
            diff_in_seconds = (datetime.datetime.utcnow() - self.clients[client_id].last_seen).total_seconds()
            if diff_in_seconds > 60:
                self.clients[client_id].connection_status = CONNECTION_STATUS.end

    def recv(self, con):
        # self.disconnect_long_last_seen()
        data = super(MessageUServer, self).recv(con)
        if len(data) <= 0:
            print("client disconnected")
            return ''
        try:
            mup_req = MessageUProtocol.MUPReqMessageWrapper(list(data))
            if mup_req.err:
                print("MessageUServer, got Request with err byte stream:", data)
                print("MessageUServer, err:", mup_req.err)
                con.send(bytes(self.err_response.bytes_arr) + Server.END_OF_PACKET)
                con.close()
                return ''
            if MessageUProtocol.MUP_REQ_MESSAGE_COD_TYPE_REGISTRETION == MessageUProtocol.MUP_REQ_MESSAGE_COD_CODE(mup_req.code):
                if self.sql_db.name_exists(mup_req.registration_payload.name.name.strip('\0')):
                    con.send(bytes(self.err_response.bytes_arr) + Server.END_OF_PACKET)
                    con.close()
                else:
                    self.inc_last_client_id()
                    client = MessageUClient(client_id=self.last_client_id, connection=con,
                                            connection_status=CONNECTION_STATUS.active, request=mup_req,
                                            request_status=REQ_STATUS.answer)
                    self.clients[client.client_id] = client
                    self.clients[client.client_id].add_client(self.sql_db)
                    threading.Thread(target=self.clients[client.client_id].serv).start()
            else:
                if self.sql_db.id_exists(mup_req.msg.header.id.id):
                    client = MessageUClient(client_id=MessageUProtocol.clientId(mup_req.msg.header.id.id), connection=con,
                                            connection_status=CONNECTION_STATUS.active, request=mup_req,
                                            request_status=REQ_STATUS.answer)
                    self.clients[client.client_id] = client
                    threading.Thread(target=self.clients[client.client_id].serv).start()
                else:
                    con.send(bytes(self.err_response.bytes_arr) + Server.END_OF_PACKET)
                    con.close()
        except MemoryError:
            print("client disconnected")

