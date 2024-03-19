import socket
import struct

# Definição da estrutura
struct_message_format = "HHHHHHHHHBBBBB"
struct_message_size = struct.calcsize(struct_message_format)

def receive_data():
    HOST = '172.20.10.5'  
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


        send_data_to_client(client_socket)


def send_data_to_client(client_socket):
    # Dados a serem enviados ao cliente ESP32
    data_to_send = struct.pack(struct_message_format, *range(14))

    client_socket.sendall(data_to_send)
    print("Dados enviados ao cliente ESP32:", list(range(14)))


if __name__ == "__main__":
    receive_data()