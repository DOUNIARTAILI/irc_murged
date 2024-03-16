import socket
import threading

# Informations de connexion au serveur IRC
server_ip = "e3r1p2.1337.ma"
# server_ip = "localhost"
server_port = 6666
server_password = "pass"
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

    irc_socket.send("USER {} 0 * Bot\r\n".format(nick).encode())
    irc_socket.send("NICK {}\r\n".format(nick).encode())
    irc_socket.send("JOIN #c1\r\n".format(nick).encode())
    irc_socket.send("JOIN #c2\r\n".format(nick).encode())
    irc_socket.send("JOIN #c3\r\n".format(nick).encode())
    irc_socket.send("JOIN #c4\r\n".format(nick).encode())
    irc_socket.send("JOIN #c5\r\n".format(nick).encode())
    irc_socket.send("JOIN #c6\r\n".format(nick).encode())
    irc_socket.send("JOIN #c7\r\n".format(nick).encode())
    irc_socket.send("JOIN #teee\r\n".format(nick).encode())

    while True:
        message = irc_socket.recv(2048).decode()
        print(message)

for nick in available_nicks:
    thread = threading.Thread(target=connect_irc, args=(nick,))
    thread.start()