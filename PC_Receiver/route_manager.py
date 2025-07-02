# 导入 time 模块，用于获取当前时间戳以判断超时
import time
# 导入 Dict 和 Any 类型，用于类型注解，增强代码可读性
from typing import Dict, Any

class RouteManager:
    # 定义常量：设备超时时间，单位秒
    DEFAULT_TIMEOUT = 8  # 超时时间（秒）

    def __init__(self):
        # 初始化路由图字典，存储每个客户端的路由表（key: src, value: RouteEntry 列表）
        self.route_graph: Dict[int, Any] = {}
        # 初始化客户端列表，存储上报路由表的节点地址
        self.clients: set = set()
        # 初始化最后活跃时间，记录每个节点的最后数据包时间
        self.last_seen: Dict[int, float] = {}

    def update_route_table(self, src: int, route_entries: list) -> None:
        # 更新路由表，标记 src 为客户端
        self.route_graph[src] = route_entries
        self.clients.add(src)
        self.last_seen[src] = time.time()
        # 更新路由表中节点的最后活跃时间
        for entry in route_entries:
            self.last_seen[entry.address] = time.time()

    def update_data_packet(self, src: int) -> None:
        # 更新数据包的最后活跃时间
        self.last_seen[src] = time.time()

    def get_node_status(self, src: int) -> tuple[str, bool]:
        # 获取节点状态和颜色
        # 返回 (color, should_remove)，color 为 "lightblue", "lightgreen", "red"，should_remove 表示是否移除
        now = time.time()
        elapsed = now - self.last_seen.get(src, 0)
        in_route_table = any(
            src in [entry.address for entry in routes]
            for other_src, routes in self.route_graph.items()
            if other_src != src
        )

        if src in self.clients:
            # 客户端节点
            if elapsed <= self.DEFAULT_TIMEOUT:
                return "lightblue", False  # 活跃客户端，蓝色
            else:
                return "red", not in_route_table  # 超时客户端，红色，移除如果不在路由表
        else:
            # 非客户端节点
            if in_route_table:
                return "lightgreen", False  # 在路由表中，绿色
            else:
                return "red", True  # 不在路由表中，红色且移除

    def get_node_colors(self) -> Dict[int, str]:
        # 返回所有节点的颜色字典
        colors = {}
        for src in self.last_seen:
            color, _ = self.get_node_status(src)
            colors[src] = color
        return colors

    def get_route_graph(self) -> Dict[int, Any]:
        # 返回路由图，供绘图使用
        return self.route_graph