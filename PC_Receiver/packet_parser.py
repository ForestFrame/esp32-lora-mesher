import struct
from dataclasses import dataclass


# ========= 常量定义 ========= #
DATA_P = 0b00000010  # 只处理这一类包


# ========= 结构体定义 ========= #

@dataclass
class PacketHeader:
    dst: int
    src: int
    type: int
    id: int
    packetSize: int


@dataclass
class RouteDataPacket:
    via: int


@dataclass
class TestData:
    sequenceNumber: int
    timestamp: int
    nodeId: int
    testType: int
    payload: bytes


# ========= 解包函数 ========= #

def parse_packet_header(data: bytes, offset=0) -> tuple[PacketHeader, int]:
    fmt = "<HHBBB"
    size = struct.calcsize(fmt)
    if len(data) < offset + size:
        raise ValueError("数据不足以解包 PacketHeader")
    values = struct.unpack_from(fmt, data, offset)
    return PacketHeader(*values), offset + size


def parse_route_data_packet(data: bytes, offset=0) -> tuple[RouteDataPacket, int]:
    fmt = "<H"
    size = struct.calcsize(fmt)
    if len(data) < offset + size:
        raise ValueError("数据不足以解包 RouteDataPacket")
    (via,) = struct.unpack_from(fmt, data, offset)
    return RouteDataPacket(via), offset + size


def parse_test_data(data: bytes, offset=0) -> tuple[TestData, int]:
    fmt = "<IIHB32s"
    size = struct.calcsize(fmt)
    if len(data) < offset + size:
        raise ValueError("数据不足以解包 TestData")
    values = struct.unpack_from(fmt, data, offset)
    return TestData(*values), offset + size


# ========= 总解析入口 ========= #

def parse_data_packet(data: bytes) -> dict:
    offset = 0

    # 1. PacketHeader
    header, offset = parse_packet_header(data, offset)
    if header.type != DATA_P:
        raise ValueError(f"不是数据包类型：{header.type}")

    # 2. RouteDataPacket
    route_data, offset = parse_route_data_packet(data, offset)

    # 3. TestData Payload
    test_data, offset = parse_test_data(data, offset)

    return {
        "header": header,
        "route": route_data,
        "test_data": test_data,
    }


# ========= 测试接口 ========= #
if __name__ == "__main__":
    raw = b'\xfd\xff<6\x02\x02\x35\xfd\xff\x00\x00\x00\x00\x996\x00\x00<6\x01\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x00'
    result = parse_data_packet(raw)
    print(result)
