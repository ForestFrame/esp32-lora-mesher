import tkinter as tk
from tkinter import ttk
import time
import threading
from typing import Dict, Any
import math

class PacketDashboard:
    def __init__(self, title: str = "ESP32 实时数据监控"):
        # GUI主线程初始化
        self.root = tk.Tk()
        self.root.title(title)

        self.frame = ttk.Frame(self.root, padding=10)
        self.frame.pack(side="left", fill="both", expand=True)

        self.canvas = tk.Canvas(self.root, bg="white", width=600, height=600)
        self.canvas.pack(side="right", fill="both", expand=True)

        # 存储每个 src 的状态和控件引用
        self._info: Dict[int, Dict[str, Any]] = {}
        self._labels: Dict[int, Dict[str, ttk.Label]] = {}
        self._lock = threading.Lock()

        # 路由图状态
        self.route_graph: Dict[int, Any] = {}

        # 启动定时更新速率的循环
        self._update_loop()

    def _update_loop(self):
        with self._lock:
            now = time.time()
            for src, data in self._info.items():
                elapsed = now - data["last_time"]
                rate = (data["count"] / elapsed) if elapsed > 0 else 0.0
                data["count"] = 0
                data["last_time"] = now
                self._labels[src]["rate"].config(text=f"速率: {rate:.2f} pkt/s")
        self.root.after(1000, self._update_loop)

    def update(self, parsed: dict):
        self.root.after(0, self._update_safe, parsed)

    def _update_safe(self, parsed: dict):
        if parsed["type"] == "test_data":
            self.update_test_data(parsed["header"], parsed["via"], parsed["payload"])
        elif parsed["type"] == "route_table":
            self.update_route_table(parsed["header"], parsed["payload"])

    def update_test_data(self, header, route, test_data):
        self._update_ui({"header": header})

    def update_route_table(self, header, route_entries):
        self._update_ui({"header": header})
        self._draw_route_graph(header.src, route_entries)

    def _update_ui(self, parsed_info: Dict[str, Any]):
        header = parsed_info.get("header")
        if header is None:
            return

        src = header.src
        pkt_type = header.type
        pkt_size = header.packetSize

        with self._lock:
            if src not in self._info:
                frame = ttk.LabelFrame(self.frame, text=f"设备 src: 0x{src:04X}", padding=8)
                frame.pack(fill="x", pady=5)

                lbl_type = ttk.Label(frame, text=f"type: {pkt_type}")
                lbl_size = ttk.Label(frame, text=f"packetSize: {pkt_size}")
                lbl_rate = ttk.Label(frame, text="速率: 0.00 pkt/s")

                for lbl in (lbl_type, lbl_size, lbl_rate):
                    lbl.pack(anchor="w")

                self._info[src] = {"count": 0, "last_time": time.time()}
                self._labels[src] = {"type": lbl_type, "size": lbl_size, "rate": lbl_rate}

            self._info[src]["count"] += 1
            self._labels[src]["type"].config(text=f"type: {pkt_type}")
            self._labels[src]["size"].config(text=f"packetSize: {pkt_size}")

    def _draw_route_graph(self, src: int, route_entries):
        # 记录上报的蓝色设备和其连接信息
        self.route_graph[src] = route_entries

        # 清除画布，重新绘制所有蓝色设备和其路由
        self.canvas.delete("all")

        # 获取所有上报的 WiFi 设备
        blue_nodes = list(self.route_graph.keys())

        # 用于确定节点的坐标，避免重叠
        layout: Dict[int, tuple[int, int]] = {}

        center_x, center_y = 50, 200
        radius = 200
        angle_step = 360 // max(len(blue_nodes), 1)

        # 第一步：画蓝色主节点
        for idx, node in enumerate(blue_nodes):
            angle_rad = math.radians(idx * angle_step)
            x = center_x + int(radius * math.cos(angle_rad))
            y = center_y + int(radius * math.sin(angle_rad))
            layout[node] = (x, y)
            self._draw_node(x, y, node, fill="lightblue")

        # 第二步：画每个主节点的绿色从节点 + 线
        for src_id, routes in self.route_graph.items():
            src_x, src_y = layout[src_id]
            spacing = 360 // max(len(routes), 1)

            for i, entry in enumerate(routes):
                child_id = entry.address
                if child_id in layout:
                    # 如果这个 child_id 也是蓝色设备，则仍然画绿色节点，但避免重叠
                    child_x, child_y = layout[child_id]
                else:
                    angle_rad = math.radians(i * spacing)
                    child_x = src_x + int(100 * math.cos(angle_rad))
                    child_y = src_y + int(100 * math.sin(angle_rad))
                    layout[child_id] = (child_x, child_y)
                    self._draw_node(child_x, child_y, child_id, fill="lightgreen")

                # 连接线
                self.canvas.create_line(src_x, src_y, child_x, child_y, fill="black")

    def _draw_node(self, x, y, addr, fill="white"):
        r = 20
        self.canvas.create_oval(x - r, y - r, x + r, y + r, fill=fill, outline="black")
        self.canvas.create_text(x, y, text=f"{addr:04X}", font=("Arial", 10))

    def run(self):
        self.root.mainloop()

