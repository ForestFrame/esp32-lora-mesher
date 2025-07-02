# 导入 tkinter 库，用于创建 GUI 界面
import tkinter as tk
# 导入 ttk 模块，提供增强的界面控件（如 LabelFrame、Label）
from tkinter import ttk
# 导入 threading 模块，用于线程安全锁，防止多线程访问冲突
import threading
# 导入 Dict 和 Any 类型，用于类型注解，增强代码可读性
from typing import Dict, Any
# 导入 RouteGraphVisualizer 类，用于绘制网络拓扑图
from route_graph_visualizer import RouteGraphVisualizer
# 导入 RouteManager 类，用于管理路由信息
from route_manager import RouteManager
# 导入 logging 模块，用于调试日志
import logging

# 配置日志记录
logging.basicConfig(level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(message)s")

# 定义 PacketDashboard 类，负责数据监控界面和逻辑
class PacketDashboard:
    # 定义常量：设备超时时间，单位秒，同步 RouteManager
    DEFAULT_TIMEOUT = 8  # 超时时间（秒）

    # 初始化方法，创建 GUI 界面并设置初始状态
    def __init__(self, title: str = "ESP32 实时数据监控"):
        # 创建主窗口（Tkinter 根窗口）
        self.root = tk.Tk()
        # 设置窗口标题，默认为 "ESP32 实时数据监控"
        self.root.title(title)

        # 创建左侧框架，用于显示设备信息（如速率、类型、大小）
        self.frame = ttk.Frame(self.root, padding=10)
        # 将框架放置在窗口左侧，填充水平和垂直方向，允许扩展
        self.frame.pack(side="left", fill="both", expand=True)

        # 创建右侧画布，用于绘制网络拓扑图，背景为白色，尺寸 800x600
        self.canvas = tk.Canvas(self.root, bg="white", width=800, height=600)
        # 将画布放置在窗口右侧，填充整个空间并允许扩展
        self.canvas.pack(side="right", fill="both", expand=True)

        # 初始化字典，存储每个设备（src）的状态信息（如字节计数）
        self._info: Dict[int, Dict[str, Any]] = {}
        # 初始化字典，存储每个设备（src）的 UI 控件引用（包括 LabelFrame）
        self._labels: Dict[int, Dict[str, Any]] = {}  # 改为 Any 以存储 LabelFrame
        # 创建线程锁，确保多线程访问 self._info 和 self._labels 时安全
        self._lock = threading.Lock()

        # 创建 RouteManager 实例，管理路由信息
        self.route_manager = RouteManager()
        # 创建 RouteGraphVisualizer 实例，负责在画布上绘制拓扑图
        self.visualizer = RouteGraphVisualizer(self.canvas)

        # 启动定时更新循环，每秒更新速率、颜色和拓扑图
        self._update_loop()

    # 定时更新方法，每秒运行一次，更新速率、颜色和拓扑图
    def _update_loop(self):
        # 使用线程锁保护共享资源访问
        with self._lock:
            # 遍历所有设备的状态信息
            to_remove = []
            for src, data in list(self._info.items()):
                # 计算速率：将过去 1 秒的字节计数除以固定 1 秒窗口
                rate = data["byte_count"] / 1.0  # 固定 1 秒窗口
                # 重置字节计数，为下一秒统计做准备
                data["byte_count"] = 0
                # 更新 UI 上该设备的速率标签
                self._labels[src]["rate"].config(text=f"速率: {rate:.2f} bytes/s")
                # 获取节点状态和颜色
                new_color, should_remove = self.route_manager.get_node_status(src)
                # 仅当颜色变化时更新，防止闪烁
                if new_color != data["color"]:
                    data["color"] = new_color
                    logging.debug(f"Node 0x{src:04X} color changed to {new_color}")
                # 标记需要移除的节点
                if should_remove:
                    to_remove.append(src)

            # 移除超时的节点
            for src in to_remove:
                if src in self._labels:
                    logging.debug(f"Removing node 0x{src:04X} from UI")
                    self._labels[src]["frame"].destroy()  # 销毁整个 LabelFrame
                    del self._labels[src]
                if src in self._info:
                    del self._info[src]

            # 调用绘图器重绘拓扑图，传递路由图和当前节点颜色
            self.visualizer.draw(self.route_manager.get_route_graph(), self.route_manager.get_node_colors())
        # 每 1000 毫秒（1 秒）调度下一次更新循环
        self.root.after(1000, self._update_loop)

    # 外部调用接口，接收解析后的数据包并调度到主线程更新 UI
    def update(self, parsed: dict):
        # 使用 after 方法将更新操作调度到 Tkinter 主线程
        self.root.after(0, self._update_safe, parsed)

    # 安全更新方法，在主线程中处理解析后的数据包
    def _update_safe(self, parsed: dict):
        # 根据数据包类型调用相应处理方法
        if parsed["type"] == "test_data":
            self.update_test_data(parsed["header"], parsed["via"], parsed["payload"])
        elif parsed["type"] == "route_table":
            self.update_route_table(parsed["header"], parsed["payload"])

    # 处理测试数据包，更新 UI 显示
    def update_test_data(self, header, route, test_data):
        # 提取 header 信息并传递给 _update_ui 方法
        self._update_ui({"header": header})
        # 更新 RouteManager 的数据包时间
        self.route_manager.update_data_packet(header.src)

    # 处理路由表数据包，更新路由图并刷新 UI
    def update_route_table(self, header, route_entries):
        # 更新 UI 显示（设备信息，如类型、大小）
        self._update_ui({"header": header}, is_client=True)
        # 更新 RouteManager 的路由表
        self.route_manager.update_route_table(header.src, route_entries)

    # 内部方法，更新设备信息和 UI 显示
    def _update_ui(self, parsed_info: Dict[str, Any], is_client: bool = False):
        # 获取数据包头信息
        header = parsed_info.get("header")
        if header is None:
            logging.warning("Received None header, skipping update")
            return

        # 提取设备源地址、数据包类型和大小
        src = header.src
        pkt_type = header.type
        pkt_size = header.packetSize

        # 过滤无效的 src（例如 0x0000 或负数）
        if src <= 0:
            logging.warning(f"Invalid src address 0x{src:04X}, skipping")
            return

        # 使用线程锁保护共享资源访问
        with self._lock:
            # 检查是否已存在 LabelFrame，避免重复创建
            if src in self._labels:
                logging.debug(f"Updating existing node 0x{src:04X}")
                # 更新类型标签
                self._labels[src]["type"].config(text=f"type: {pkt_type}")
                # 更新大小标签
                self._labels[src]["size"].config(text=f"packetSize: {pkt_size}")
                # 更新字节计数
                self._info[src]["byte_count"] += pkt_size
                # 更新颜色（仅客户端设为蓝色）
                if is_client and self._info[src]["color"] != "lightblue":
                    self._info[src]["color"] = "lightblue"
                    logging.debug(f"Node 0x{src:04X} set to client (lightblue)")
            else:
                # 创建新设备框
                logging.debug(f"Creating new node 0x{src:04X}")
                # 创建一个标签框架，显示设备地址
                frame = ttk.LabelFrame(self.frame, text=f"设备 src: 0x{src:04X}", padding=8)
                frame.pack(fill="x", pady=5)

                # 创建显示数据包类型的标签
                lbl_type = ttk.Label(frame, text=f"type: {pkt_type}")
                # 创建显示数据包大小的标签
                lbl_size = ttk.Label(frame, text=f"packetSize: {pkt_size}")
                # 创建显示速率的标签，初始为 0.00 bytes/s
                lbl_rate = ttk.Label(frame, text="速率: 0.00 bytes/s")

                # 将所有标签依次添加到框架中，左对齐
                for lbl in (lbl_type, lbl_size, lbl_rate):
                    lbl.pack(anchor="w")

                # 初始化设备状态信息，初始颜色根据是否为客户端
                self._info[src] = {
                    "byte_count": pkt_size,  # 初始化字节计数
                    "color": "lightblue" if is_client else "lightgreen"
                }
                # 存储 UI 控件引用，包括 LabelFrame
                self._labels[src] = {
                    "frame": frame,  # 存储 LabelFrame 以便销毁
                    "type": lbl_type,
                    "size": lbl_size,
                    "rate": lbl_rate
                }

            # 更新颜色（仅客户端设为蓝色）
            if is_client and self._info[src]["color"] != "lightblue":
                self._info[src]["color"] = "lightblue"
                logging.debug(f"Node 0x{src:04X} updated to client (lightblue)")

    # 启动 GUI 主循环，运行 Tkinter 应用程序
    def run(self):
        self.root.mainloop()
