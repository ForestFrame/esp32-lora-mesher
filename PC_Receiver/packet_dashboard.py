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

                lbl_type = ttk.Label(frame, text = f"type: {pkt_type}")
                lbl_size = ttk.Label(frame, text=f"packetSize: {pkt_size}")
                lbl_rate = ttk.Label(frame, text="速率: 0.00 pkt/s")

                for lbl in (lbl_type, lbl_size, lbl_rate):
                    lbl.pack(anchor="w")

                self._info[src] = {"count": 0, "last_time": time.time()}
                self._labels[src] = {"type": lbl_type, "size": lbl_size, "rate": lbl_rate}

            self._info[src]["count"] += 1
            self._labels[src]["type"].config(text=f"type: {pkt_type}")
            self._labels[src]["size"].config(text=f"packetSize: {pkt_size}")

    def _get_circle_edge_points(self, x1: float, y1: float, x2: float, y2: float, r: float) -> tuple[
        float, float, float, float]:
        """计算两节点圆周交点坐标，r 为节点半径"""
        dx = x2 - x1
        dy = y2 - y1
        distance = math.sqrt(dx ** 2 + dy ** 2)

        if distance == 0:
            return x1, y1, x2, y2  # 防止除零错误

        unit_x = dx / distance
        unit_y = dy / distance

        start_x = x1 + r * unit_x
        start_y = y1 + r * unit_y
        end_x = x2 - r * unit_x
        end_y = y2 - r * unit_y

        return start_x, start_y, end_x, end_y

    def _draw_route_graph(self, src: int, route_entries):
        # 记录上报的蓝色设备和其连接信息
        self.route_graph[src] = route_entries

        # 清除画布，重新绘制
        self.canvas.delete("all")

        # 获取所有上报的节点（蓝色主节点）
        blue_nodes = list(self.route_graph.keys())

        # 用于确定节点的坐标
        layout: Dict[int, tuple[int, int]] = {}
        center_x, center_y = 50, 200  # 移动中心到画布中间
        radius = 200
        angle_step = 360 // max(len(blue_nodes), 1)

        # 第一步：绘制蓝色主节点
        for idx, node in enumerate(blue_nodes):
            angle_rad = math.radians(idx * angle_step)
            x = center_x + int(radius * math.cos(angle_rad))
            y = center_y + int(radius * math.sin(angle_rad))
            layout[node] = (x, y)
            self._draw_node(x, y, node, fill="lightblue")

        # 第二步：收集所有连接边
        edges = []
        for src_id, routes in self.route_graph.items():
            for entry in routes:
                to_node = entry.address
                via_node = entry.via
                if via_node == to_node:
                    edges.append((src_id, to_node))
                else:
                    edges.append((via_node, to_node))

        # 第三步：绘制从节点和连接线
        node_index = 0  # 用于动态分配从节点位置
        for from_node, to_node in edges:
            # 如果 from_node 不在 layout 中，分配坐标
            if from_node not in layout:
                # 从最后一个主节点的附近开始，沿链式方向偏移
                parent_x, parent_y = layout.get(list(self.route_graph.keys())[-1], (center_x, center_y))
                angle_rad = math.radians(node_index * 45)  # 每 45 度一个节点
                offset_distance = 100 + (node_index % 3) * 50  # 不同距离避免重叠
                x = parent_x + int(offset_distance * math.cos(angle_rad))
                y = parent_y + int(offset_distance * math.sin(angle_rad))
                layout[from_node] = (x, y)
                self._draw_node(x, y, from_node, fill="lightgreen")
                node_index += 1

            # 如果 to_node 不在 layout 中，分配坐标（基于 from_node）
            if to_node not in layout:
                from_x, from_y = layout[from_node]
                # 沿 from_node 到 to_node 的方向偏移，模拟链式结构
                angle_rad = math.radians(node_index * 45)
                offset_distance = 100 + (node_index % 3) * 50
                x = from_x + int(offset_distance * math.cos(angle_rad))
                y = from_y + int(offset_distance * math.sin(angle_rad))
                layout[to_node] = (x, y)
                self._draw_node(x, y, to_node, fill="lightgreen")
                node_index += 1

            # 绘制连接线（圆周到圆周）
            from_x, from_y = layout[from_node]
            to_x, to_y = layout[to_node]
            r = 20  # 节点半径
            start_x, start_y, end_x, end_y = self._get_circle_edge_points(from_x, from_y, to_x, to_y, r)
            self.canvas.create_line(start_x, start_y, end_x, end_y, fill="black")

    def _draw_node(self, x, y, addr, fill="white"):
        r = 20
        self.canvas.create_oval(x - r, y - r, x + r, y + r, fill=fill, outline="black")
        self.canvas.create_text(x, y, text=f"{addr:04X}", font=("Arial", 10))

    def run(self):
        self.root.mainloop()