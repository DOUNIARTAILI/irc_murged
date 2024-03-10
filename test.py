import socket
import threading

# Informations de connexion au serveur IRC
server_ip = "e1r3p2.1337.ma"
# server_ip = "localhost"
server_port = 6669
server_password = "test"
# server_password = "zarda"

# Liste de pseudos disponibles
available_nicks = []


for i in range(1, 40):
    nick = "client_bot{}".format(i)
    available_nicks.append(nick)

def connect_irc(nick):
    irc_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    irc_socket.connect((server_ip, server_port))

    irc_socket.send("PASS {}\r\n".format(server_password).encode())

    irc_socket.send("USER {} 0 * :Client Bot\r\n".format(nick).encode())
    irc_socket.send("NICK {}\r\n".format(nick).encode())
    irc_socket.send("JOIN #teee\r\n".format(nick).encode())

    while True:
        message = irc_socket.recv(2048).decode()
        print(message)

for nick in available_nicks:
    thread = threading.Thread(target=connect_irc, args=(nick,))
    thread.start()