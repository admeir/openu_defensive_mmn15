import os
import sqlite3
import threading


def get_port():
    with open('port.info', 'r') as port_info:
        return port_info.read()


class IdSync:
    def __init__(self, sql):
        self.sql =sql
        self.free_message_ids = [None for _ in range(0xffff)]
        self.free_message_ids[0] = list(range(0xffff))
        self.sync_free_message_ids()

    def create_free_message_ids_page(self, page):
        if page < 0xffff:
            if self.free_message_ids[page] is None:
                self.free_message_ids[page] = list(range(0xffff))
            return 1
        else:
            print("ERROR: messages id was to big")
            return 0

    def sync_free_message_ids(self):
        for _id in self.sql.get_ids('messages'):
            page = _id >> 32
            if self.create_free_message_ids_page(page):
                del self.free_message_ids[page][self.free_message_ids[page].index(_id)]

    def allocate_message_id(self):
        page = 0
        while self.free_message_ids[page] is not None and len(self.free_message_ids[page]) == 0:
            page += 1
        print("page:", page)
        if self.create_free_message_ids_page(page):
            _id = self.free_message_ids[page][0]
            del self.free_message_ids[page][self.free_message_ids[page].index(_id)]
            print("id:", _id)
            return ((page << 32) | _id)
        else:
            print("ERROR, there no left message ids")
            return None

    def free_message_id(self, _id):
        page = _id >> 32
        _id = self.free_message_ids[page]
        self.free_message_ids[page].append(_id & 0xffffffff)


class SQL:
    def __init__(self, db_name: str = "server.db"):
        self.connection = sqlite3.connect(db_name)
        # self.my_cursor =
        self.db_name = db_name
        self.sem = threading.Lock()
        self.create_table_if_not_exists('clients', {'name': 'ID', "type": "varchar(32)"},
                                        [{'name': 'Name', "type": "varchar(255)", "NOT NULL": ""},
                                         {'name': 'PublicKey', "type": "varchar(160)", "NOT NULL": ""},
                                         {'name': 'LastSeen', "type": "datetime", "NOT NULL": ""}])

        self.create_table_if_not_exists('messages', {'name': 'ID', "type": "integer"},
                                        [{'name': 'ToClient', "type": "varchar(16)", "NOT NULL": ""},
                                         {'name': 'FromClient', "type": "varchar(16)", "NOT NULL": ""},
                                         {'name': 'Type', "type": "varchar(1)", "NOT NULL": ""},
                                         {'name': 'Content', "type": "NUMBER(1)", "NOT NULL": ""}])
        self.id_sync = IdSync(self)

    def write(self, cmd, *args):
        ret = None
        with self.connection:
            print(cmd, args)
            ret = self.connection.execute(cmd, *args)
            self.connection.commit()
        return ret

    def read(self, cmd, *args):
        ret = []
        with self.connection:
            print(cmd, args)
            ret = self.connection.execute(cmd, *args)
            ret = ret.fetchall()
        return list(ret)

    def table_exists(self, table_name: str = ""):
        ret = self.read(f"SELECT name FROM sqlite_master WHERE type='table' AND name='{table_name}';")
        return ret

    def create_table_if_not_exists(self, table_name: str = "",
                                   primary_key={"name": "var1", "type": "integer"},
                                   keys=[
                                       {"name": "var2", "type": "varchar", "NOT NULL": "NOT NULL"},
                                       {"name": "var3", "type": "integer", "NOT NULL": ""}
                                   ],
                                   foreign_key=[],
                                   preferences=""):
        cmd = \
            f"""CREATE TABLE IF NOT EXISTS {table_name} (
	{primary_key['name']} {primary_key['type']} PRIMARY KEY,
	{','.join([f'{x["name"]} {x["type"]} {x["NOT NULL"]}' for x in keys])}
	{f'FOREIGN KEY ({",".join(foreign_key)}) REFERENCES {preferences}' if len(foreign_key) else ''}
);"""
        return self.write(cmd)

    def add_client(self, client_id, name, public_key, last_seen):
        cmd = "insert or replace into clients (ID, Name, PublicKey, LastSeen) values (?, ?, ?, ?);"
        return self.write(cmd, (client_id, name, public_key, last_seen,))

    def update_last_seen(self, client_id, last_seen):
        return self.write("UPDATE clients SET LastSeen = ? WHERE ID = ?;", (last_seen, client_id, ))

    def name_exists(self, name):
        return len(self.read("SELECT Name FROM clients WHERE Name = ?;", (name,)))

    def id_exists(self, c_id):
         return len(self.read("SELECT ID FROM clients WHERE ID = ?;", (c_id,)))

    def table_client_war_to_dict(self, client_war):
        return {client_war[0]: {'name': client_war[1],
                                'public_key': client_war[2],
                                'last_seen': client_war[3]}
                }

    def get_clients(self):
        clients_dict = dict()
        clients = self.read("SELECT * FROM clients;")
        for client in clients:
            clients_dict.update(self.table_client_war_to_dict(client))
        return clients_dict

    def get_client(self, id):
        return self.table_client_war_to_dict(self.read("SELECT * FROM clients WHERE ID = ?;", (id,))[0])

    def get_ids(self, table):
        ids = self.read(f"SELECT ID FROM {table};")
        return [_id[0] for _id in ids]

    def get_last_id(self, table):
        ids = self.read(f"SELECT ID FROM {table};")
        return ids[-1][0] if len(ids) else None

    def add_message(self, from_client, to_client, _type, content):
        message_id = self.id_sync.allocate_message_id()
        if message_id is not None:
            cmd = "insert into messages (ID, ToClient, FromClient, Type, Content) values (?, ?, ?, ?, ?);"
            self.write(cmd, (message_id, from_client, to_client, _type, content,))
        return message_id

    def get_messages(self, client_id):
        return self.read("SELECT * FROM messages WHERE ToClient = ?;", (client_id,))

import datetime
if __name__ == '__main__':
    sql_db = SQL()
    sql_db.add_client("10000000000000000000000000000000",
                      "sdflkdlsfl",
                      "111257e427c3aa5bc1fb9322dca65860af4469e090723f5adcd401722448f1e73bfb61bd1e549c2a265e4e4f1a49fd98398557a273e4f1bc74d660c9ea771c98c2d38f8f8de33433365abd7a48ffb8d961be2740a38b6423416db14091662f363a916aac7ee2cb5e1db315c8de12f7af77dc49436d5c449c7247cbbb8808181287813008b81305111df78648862a96d309d8130",
                      datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S'))

    sql_db.add_client("20000000000000000000000000000000",
                      "Nudelman Beatriz",
                      "111257e427c3aa5bc1fb9322dca65860af4469e090723f5adcd401722448f1e73bfb61bd1e549c2a265e4e4f1a49fd98398557a273e4f1bc74d660c9ea771c98c2d38f8f8de33433365abd7a48ffb8d961be2740a38b6423416db14091662f363a916aac7ee2cb5e1db315c8de12f7af77dc49436d5c449c7247cbbb8808181287813008b81305111df78648862a96d309d8130",
                      datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S'))
    sql_db.update_last_seen("10000000000000000000000000000000",
                            "1021-10-16 09:07:32")

    print(sql_db.name_exists("sdflkdlsfl"))
    print(sql_db.id_exists("30000000000000000000000000000000"))
    print(sql_db.get_last_id('clients'))
    print(sql_db.get_client(sql_db.get_last_id('clients')))
    print(sql_db.get_clients())
    print(sql_db.get_messages("30000000000000000000000000000000"))