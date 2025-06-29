import time
import threading
import tkinter as tk
from tkinter import ttk
from typing import Dict, Any

class PacketDashboard:
    def __init__(self, title: str = "ESP32 实时数据监控"):
        # GUI主线程初始化
        self.root = tk.Tk()
        self.root.title(title)
        self.frame = ttk.Frame(self.root, padding=10)
        self.frame.pack(fill="both", expand=True)

        # 存储每个 src 的状态和控件引用
        self._info: Dict[int, Dict[str, Any]] = {}
        self._labels: Dict[int, Dict[str, ttk.Label]] = {}
        self._lock = threading.Lock()

        # 启动定时更新速率的循环
        self._update_loop()

    def _update_loop(self):
        """定时（1秒）更新所有速率显示，并继续下一个周期"""
        with self._lock:
            now = time.time()
            for src, data in self._info.items():
                elapsed = now - data["last_time"]
                rate = (data["count"] / elapsed) if elapsed > 0 else 0.0
                # 重置统计
                data["count"] = 0
                data["last_time"] = now
                # 更新速率标签
                self._labels[src]["rate"].config(text=f"速率: {rate:.2f} pkt/s")
        # 1秒后再次调用自己，循环执行
        self.root.after(1000, self._update_loop)

    def update(self, parsed_info: Dict[str, Any]):
        """
        外部接口：收到数据后调用。
        通过 root.after 异步安全更新GUI。
        """
        self.root.after(0, lambda: self._update_ui(parsed_info))

    def _update_ui(self, parsed_info: Dict[str, Any]):
        """实际更新GUI控件，主线程调用"""
        header = parsed_info.get("header")
        if header is None:
            return

        src = header.src
        pkt_type = header.type
        pkt_size = header.packetSize

        with self._lock:
            if src not in self._info:
                # 新设备，创建面板和标签
                frame = ttk.LabelFrame(self.frame, text=f"设备 src: 0x{src:04X}", padding=8)
                frame.pack(fill="x", pady=5)

                lbl_type = ttk.Label(frame, text=f"type: {pkt_type}")
                lbl_size = ttk.Label(frame, text=f"packetSize: {pkt_size}")
                lbl_rate = ttk.Label(frame, text="速率: 0.00 pkt/s")

                for lbl in (lbl_type, lbl_size, lbl_rate):
                    lbl.pack(anchor="w")

                self._info[src] = {
                    "count": 0,
                    "last_time": time.time(),
                }
                self._labels[src] = {
                    "type": lbl_type,
                    "size": lbl_size,
                    "rate": lbl_rate,
                }

            # 更新统计计数和标签显示
            self._info[src]["count"] += 1
            self._labels[src]["type"].config(text=f"type: {pkt_type}")
            self._labels[src]["size"].config(text=f"packetSize: {pkt_size}")

    def run(self):
        """启动GUI，阻塞调用，必须在主线程运行"""
        self.root.mainloop()
