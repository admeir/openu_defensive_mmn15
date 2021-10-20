import socket


class Server:
    END_OF_PACKET = b"MEIR_END_OF_PACKET\0"
    BUFFER_SIZE = 1024
    def __str__(self):
        return f"""Server:
        serv port:\t{self.port}
        """

    def __del__(self):
        self.sock.close()

    def __init__(self, port: int = 8080):
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.clients = {}
        self.connected = True

    def serv(self):
        self.sock.bind(("localhost", int(self.port)))
        while self.connected:
            self.serv_foo()

    def serv_foo(self):
        self.sock.listen()
        con, _ = self.accept_connection()
        self.recv(con)

    def accept_connection(self):
        print('Server, wait for Connection')
        con, addr = self.sock.accept()
        print('Client Connected by', addr)
        # con.send(b"Hello From Server" + Server.END_OF_PACKET)
        return con, addr

    def recv(self, con):
        return con.recv(Server.BUFFER_SIZE)

