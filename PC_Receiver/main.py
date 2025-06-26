import socket

UDP_IP = "0.0.0.0"  # 监听所有IP
UDP_PORT = 8080

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening for UDP on port {UDP_PORT}...")

while True:
    data, addr = sock.recvfrom(1024)  # 缓冲区大小
    print(f"Received from {addr}: {data}")
