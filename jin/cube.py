import struct

W = H = 32
# BGR 顺序
header = struct.pack('<BBBHHBHHHHBB',
    0,          # id length
    0,          # color map type
    2,          # image type: uncompressed true-color
    0, 0, 0,    # color map spec
    0, 0,       # x origin, y origin
    W, H,
    24,         # bits per pixel
    0           # descriptor (bottom-left origin)
)

data = bytearray()
for y in range(H):
    for x in range(W):
        # 一个棋盘格 + 渐变，方便肉眼确认贴图生效
        c = 255 if ((x // 8) + (y // 8)) % 2 == 0 else 64
        r = c
        g = (x * 255) // (W - 1)
        b = (y * 255) // (H - 1)
        data += bytes((b, g, r))   # TGA 是 BGR

with open('cube.tga', 'wb') as f:
    f.write(header)
    f.write(data)

print("cube.tga written:", 18 + len(data), "bytes")