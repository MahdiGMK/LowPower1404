def calcTotal(stream: bytes) -> int:
    lastDig = 0
    res = 0
    for b in stream:
        for j in range(8):
            dig = b & 1
            res += dig != lastDig
            lastDig = dig
            b >>= 1;
    return res
def getBit(stream: bytes, ind: int) -> int:
    if ind >= len(stream) * 8:
        return 0
    if (stream[ind // 8] & (1 << ind % 8)) == 0:
        return 0
    return 1
def pack(bits: list) -> int:
    res = 0
    for i in range(len(bits)):
        if bits[i]:
            res += 1 << i
    return res
def extract(stream: bytes, period: int, offset: int) -> bytes:
    res = bytearray()
    while offset < len(stream) * 8:
        bits = []
        for i in range(8):
            bits.append(getBit(stream,offset))
            offset += period
        res.append(pack(bits))
    return bytes(res)
def toParallelBus(stream: bytes, numLanes :int) -> list[bytes]:
    res = []
    for i in range(numLanes):
        res.append(extract(stream, numLanes, i))
    return res

assert(extract(b'\x10', 1,0) == b'\x10')
assert(extract(b'\x10', 1,1) == b'\x08')
assert(extract(b'\x10', 1,2) == b'\x04')
assert(extract(b'\x10', 2,0) == b'\x04')
assert(extract(b'\x10', 3,0) == b'\x00')
assert(extract(b'\x10', 3,1) == b'\x02')
assert(extract(b'\x10', 4,0) == b'\x02')
assert(extract(b'\x01\x23\x45\x67', 4,0) == b'\x55')

assert(calcTotal(b'\x80\x01\x00\x00') == 2)
assert(calcTotal(b'\x80\x81\x01\x00') == 4)
assert(calcTotal(b'\x80\x99\x01\x00') == 6)
