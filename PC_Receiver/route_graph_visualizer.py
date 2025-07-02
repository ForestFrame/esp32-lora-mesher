# 导入 tkinter 库，用于创建画布和绘制图形
import tkinter as tk
# 导入 Dict 和 Any 类型，用于类型注解
from typing import Dict, Any
# 导入 math 模块，用于计算节点坐标
import math

class RouteGraphVisualizer:
    # 初始化方法，接收画布对象
    def __init__(self, canvas: tk.Canvas):
        self.canvas = canvas

    def _get_circle_edge_points(self, x1: float, y1: float, x2: float, y2: float, r: float) -> tuple[float, float, float, float]:
        # 计算两节点圆周交点坐标，r 为节点半径
        dx = x2 - x1
        dy = y2 - y1
        distance = math.sqrt(dx ** 2 + dy ** 2)

        # 防止除零错误
        if distance == 0:
            return x1, y1, x2, y2

        unit_x = dx / distance
        unit_y = dy / distance

        start_x = x1 + r * unit_x
        start_y = y1 + r * unit_y
        end_x = x2 - r * unit_x
        end_y = y2 - r * unit_y

        return start_x, start_y, end_x, end_y

    def _draw_node(self, x: float, y: float, addr: int, fill: str = "white"):
        # 绘制节点（圆形），显示地址
        r = 20
        self.canvas.create_oval(x - r, y - r, x + r, y + r, fill=fill, outline="black")
        self.canvas.create_text(x, y, text=f"{addr:04X}", font=("Arial", 10))

    def draw(self, route_graph: Dict[int, Any], node_colors: Dict[int, str]):
        # 绘制路由拓扑图，基于路由图和节点颜色
        self.canvas.delete("all")

        # 获取所有客户端节点（蓝色或红色）
        blue_nodes = list(route_graph.keys())

        # 用于确定节点的坐标
        layout: Dict[int, tuple[int, int]] = {}
        center_x, center_y = 400, 300  # 画布中心，适配 800x600
        radius = 150
        angle_step = 360 // max(len(blue_nodes), 1)

        # 第一步：绘制所有客户端节点（蓝色或红色）
        for idx, node in enumerate(blue_nodes):
            angle_rad = math.radians(idx * angle_step)
            x = center_x + int(radius * math.cos(angle_rad))
            y = center_y + int(radius * math.sin(angle_rad))
            layout[node] = (x, y)
            self._draw_node(x, y, node, fill=node_colors.get(node, "lightblue"))

        # 第二步：收集所有连接边
        edges = set()
        for src_id, routes in route_graph.items():
            for entry in routes:
                to_node = entry.address
                via_node = entry.via
                # 如果 to_node 是客户端，检查双向连接
                if to_node in blue_nodes:
                    if src_id in [e.address for e in route_graph.get(to_node, [])]:
                        edge = tuple(sorted([src_id, to_node]))
                        edges.add(edge)
                # 处理非客户端节点的连接
                elif via_node == to_node:
                    edges.add((src_id, to_node))
                else:
                    edges.add((via_node, to_node))

        # 第三步：绘制普通节点（绿色）和连接线
        node_index = 0
        for from_node, to_node in edges:
            # 为 from_node 分配坐标（如果是绿色节点）
            if from_node not in layout and from_node in node_colors:
                parent_x, parent_y = layout.get(list(blue_nodes)[-1] if blue_nodes else (center_x, center_y), (center_x, center_y))
                angle_rad = math.radians(node_index * 45)
                offset_distance = 100 + (node_index % 3) * 50
                x = parent_x + int(offset_distance * math.cos(angle_rad))
                y = parent_y + int(offset_distance * math.sin(angle_rad))
                while any(math.hypot(x - px, y - py) < 50 for px, py in layout.values()):
                    node_index += 1
                    angle_rad = math.radians(node_index * 45)
                    x = parent_x + int(offset_distance * math.cos(angle_rad))
                    y = parent_y + int(offset_distance * math.sin(angle_rad))
                layout[from_node] = (x, y)
                self._draw_node(x, y, from_node, fill=node_colors.get(from_node, "lightgreen"))
                node_index += 1

            # 为 to_node 分配坐标（如果是绿色节点）
            if to_node not in layout and to_node in node_colors:
                from_x, from_y = layout[from_node]
                angle_rad = math.radians(node_index * 45)
                offset_distance = 100 + (node_index % 3) * 50
                x = from_x + int(offset_distance * math.cos(angle_rad))
                y = from_y + int(offset_distance * math.sin(angle_rad))
                while any(math.hypot(x - px, y - py) < 50 for px, py in layout.values()):
                    node_index += 1
                    angle_rad = math.radians(node_index * 45)
                    x = from_x + int(offset_distance * math.cos(angle_rad))
                    y = from_y + int(offset_distance * math.sin(angle_rad))
                layout[to_node] = (x, y)
                self._draw_node(x, y, to_node, fill=node_colors.get(to_node, "lightgreen"))
                node_index += 1

            # 绘制连接线（圆周到圆周）
            from_x, from_y = layout[from_node]
            to_x, to_y = layout[to_node]
            r = 20
            start_x, start_y, end_x, end_y = self._get_circle_edge_points(from_x, from_y, to_x, to_y, r)
            self.canvas.create_line(start_x, start_y, end_x, end_y, fill="black")