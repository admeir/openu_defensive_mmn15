from server.utils import get_port
from server.MassageUServer import MessageUServer


def run_server():
    server = MessageUServer(get_port())
    print(server)
    server.serv()


if __name__ == '__main__':
    run_server()

