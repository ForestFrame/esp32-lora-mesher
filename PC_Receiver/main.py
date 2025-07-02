import socket
import threading
from packet_parser import parse_packet
from packet_dashboard import PacketDashboard

def udp_receiver(dashboard):
    udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp.bind(("0.0.0.0", 8080))

    print("UDP 上位机监听中：0.0.0.0:8080")
    while True:
        data, addr = udp.recvfrom(1024)
        print(' '.join(f'{b:02x}' for b in data))
        try:
            parsed = parse_packet(data)
            print(parsed)
            print(f"[{parsed['type']}] from {addr}")
            dashboard.update(parsed)
        except Exception as e:
            print(f"解析失败: {e}")

if __name__ == "__main__":

    dashboard = PacketDashboard()

    recv_thread = threading.Thread(target=udp_receiver, args=(dashboard,), daemon=True)
    recv_thread.start()

    dashboard.run()  # 主线程启动Tkinter主循环

