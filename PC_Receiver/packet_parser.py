import struct
from dataclasses import dataclass


# ========= 常量定义 ========= #
DATA_P = 0b00000010
ROUTE_TABLE_P = 0b00000110

# ========= 结构体定义 ========= #

@dataclass
class PacketHeader:
    dst: int
    src: int
    type: int
    packetSize: int

@dataclass
class RouteDataPacket:
    via: int

@dataclass
class TestData:
    sequenceNumber: int
    testType: int
    payload: bytes

@dataclass
class RouteEntry:
    address: int
    via: int
    metric: int
    role: int

# ========= 解包函数 ========= #

def parse_packet_header(data: bytes, offset=0) -> tuple[PacketHeader, int]:
    fmt = "<HHBB"
    size = struct.calcsize(fmt)
    if len(data) < offset + size:
        raise ValueError("数据不足以解包 PacketHeader")
    values = struct.unpack_from(fmt, data, offset)
    return PacketHeader(*values), offset + size


def parse_route_data_packet(data: bytes, offset=0) -> tuple[RouteDataPacket, int]:
    fmt = "<H"  # uint16_t -> 2字节
    size = struct.calcsize(fmt)
    if len(data) < offset + size:
        raise ValueError("数据不足以解包 RouteDataPacket")
    (via,) = struct.unpack_from(fmt, data, offset)
    return RouteDataPacket(via), offset + size


def parse_test_data(data: bytes, offset: int, payload_len: int) -> tuple[TestData, int]:
    header_fmt = "<IB"
    header_size = struct.calcsize(header_fmt)  # 5字节

    if payload_len < header_size:
        raise ValueError("payload长度不足以解包TestData头部")

    seq, testType = struct.unpack_from(header_fmt, data, offset)
    offset += header_size

    payload = data[offset : offset + (payload_len - header_size)]
    offset += len(payload)

    return TestData(seq, testType, payload), offset


def parse_route_entry(data: bytes, offset=0) -> tuple[RouteEntry, int]:
    fmt = "<HHBB"
    size = struct.calcsize(fmt)
    if len(data) < offset + size:
        raise ValueError("数据不足以解包 RouteEntry")
    values = struct.unpack_from(fmt, data, offset)
    return RouteEntry(*values), offset + size

# ========= 主入口 ========= #

def parse_packet(data: bytes) -> dict:
    offset = 0
    header, offset = parse_packet_header(data, offset)
    route_data, offset = parse_route_data_packet(data, offset)

    result = {
        "header": header,
        "via": route_data.via,  # 新增这一行
        "type": None,
        "payload": None
    }

    total_packet_len = header.packetSize  # 整包长度（包含头）
    header_len = offset  # 已经读取的头部长度，6字节
    payload_len = total_packet_len - header_len

    if len(data) < total_packet_len:
        raise ValueError(f"数据长度不足，期望{total_packet_len}字节，实际{len(data)}字节")

    if header.type == DATA_P:
        test_data, new_offset = parse_test_data(data, offset, payload_len)
        result["type"] = "test_data"
        result["payload"] = test_data
        offset = new_offset

    elif header.type == ROUTE_TABLE_P:
        route_entries = []
        entry_size = struct.calcsize("<HHBB")
        if payload_len % entry_size != 0:
            raise ValueError("路由表长度不是entry_size的整数倍")
        num_entries = payload_len // entry_size

        for _ in range(num_entries):
            entry, offset = parse_route_entry(data, offset)
            route_entries.append(entry)

        result["type"] = "route_table"
        result["payload"] = route_entries

    else:
        raise ValueError(f"未知数据包类型: {header.type}")

    return result


