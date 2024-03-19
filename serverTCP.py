import socket
import struct

# Definição da estrutura
struct_message_format = "HHHHHHHHHBBBBB"
struct_message_size = struct.calcsize(struct_message_format)

def receive_data():
    HOST = '192.168.100.13'  
    PORT = 62445            

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((HOST, PORT))
    server_socket.listen(1)

    print(f"Servidor TCP esperando conexão em {HOST}:{PORT}")

    while True:
        client_socket, client_address = server_socket.accept()

        data = client_socket.recv(struct_message_size)
        if not data:
            break

        unpacked_data = struct.unpack(struct_message_format, data)
        print(unpacked_data)


if __name__ == "__main__":
    receive_data()