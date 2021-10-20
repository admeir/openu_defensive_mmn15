import os
import sqlite3
import threading


def get_port():
    with open('port.info', 'r') as port_info:
        return port_info.read()


class SQL:
    def __init__(self, db_name: str = "server.db"):
        self.connection = sqlite3.connect(db_name)
        self.my_cursor = self.connection.cursor()
        self.db_name = db_name
        self.sem = threading.Semaphore()
        self.create_table_if_not_exists('clients', {'name': 'ID', "type": "varchar(32)"},
                                        [{'name': 'Name', "type": "varchar(255)", "NOT NULL": ""},
                                         {'name': 'PublicKey', "type": "varchar(160)", "NOT NULL": ""},
                                         {'name': 'LastSeen', "type": "datetime", "NOT NULL": ""}])

        self.create_table_if_not_exists('messages', {'name': 'ID', "type": "integer"},
                                        [{'name': 'ToClient', "type": "varchar(16)", "NOT NULL": ""},
                                         {'name': 'FromClient', "type": "varchar(16)", "NOT NULL": ""},
                                         {'name': 'Type', "type": "varchar(1)", "NOT NULL": ""},
                                         {'name': 'Content', "type": "NUMBER(1)", "NOT NULL": ""}])

    def write(self, cmd, *args):
        print(cmd, args)
        with self.sem:
            ret = self.my_cursor.execute(cmd, *args)
            self.connection.commit()
            return ret

    def read(self, cmd, *args):
        print(cmd, args)
        with self.sem:
            return list(self.my_cursor.execute(cmd, *args).fetchall())

    def fetch_call(*args, **kwargs):
        foo = args[0]

        def do_foo(self, *args, **kwargs):
            x = foo(self, *(args), **kwargs)
            return self.my_cursor.fetchall()

        return do_foo

    @fetch_call
    def table_exists(self, table_name: str = ""):
        ret = self.read(f"SELECT name FROM sqlite_master WHERE type='table' AND name='{table_name}';")
        return ret

    @fetch_call
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
        return self.my_cursor.execute(cmd)

    def add_client(self, client_id, name, public_key, last_seen):
        cmd = "insert or replace into clients (ID, Name, PublicKey, LastSeen) values (?, ?, ?, ?);"
        return self.write(cmd, (client_id, name, public_key, last_seen,))

    def update_last_seen(self, client_id, last_seen):
        cmd = \
            f"""UPDATE clients
SET LastSeen = "{last_seen}"
WHERE ID = "{client_id}";
;"""
        return self.write(cmd)

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

    def get_last_id(self):
        ids = self.read("SELECT ID FROM clients;")
        return ids[-1][0] if len(ids) else None


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
    print(sql_db.get_last_id())
    print(sql_db.get_client(sql_db.get_last_id()))
    print(sql_db.get_clients())