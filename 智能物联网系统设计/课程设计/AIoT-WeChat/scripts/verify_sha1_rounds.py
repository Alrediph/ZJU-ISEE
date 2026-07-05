#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
详细调试 SHA-1 计算过程（Python 版本）
"""

def rotl(n, s):
    """32-bit left rotation"""
    return ((n << s) | (n >> (32 - s))) & 0xFFFFFFFF

print("===== SHA-1 Step-by-Step Calculation (Python) =====")

# 测试 "abc"
padded = [
    0x61626380, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x18
]

print("Input 32-bit words:", [hex(w) for w in padded])

# 初始 H 值
H = [0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0]
print("\nInitial H:")
for i, h in enumerate(H):
    print(f"  H[{i}]: {h:08x}")

W = [0] * 80

# 处理第一个（也是唯一的）block
print("\nProcessing block 0:")

# 初始化 a, b, c, d, e（在循环外部）
a, b, c, d, e = H

for j in range(80):
    if j < 16:
        W[j] = padded[j]
    else:
        W[j] = rotl(W[j-3] ^ W[j-8] ^ W[j-14] ^ W[j-16], 1)

    if j < 20:
        f = (b & c) | (~b & d)
        k = 0x5A827999
    elif j < 40:
        f = b ^ c ^ d
        k = 0x6ED9EBA1
    elif j < 60:
        f = (b & c) | (b & d) | (c & d)
        k = 0x8F1BBCDC
    else:
        f = b ^ c ^ d
        k = 0xCA62C1D6

    t = (rotl(a, 5) + f + e + k + W[j]) & 0xFFFFFFFF

    # 打印前 5 步
    if j < 5:
        print(f"\n  Round {j}:")
        print(f"    W[{j}]: {W[j]:08x}")
        print(f"    a: {a:08x} b: {b:08x} c: {c:08x} d: {d:08x} e: {e:08x}")
        print(f"    f: {f:08x} k: {k:08x}")
        print(f"    t: {t:08x}")

    e = d
    d = c
    c = rotl(b, 30)
    b = a
    a = t

    if j < 5:
        print(f"    After: a: {a:08x} b: {b:08x} c: {c:08x} d: {d:08x} e: {e:08x}")

print("\nAfter 80 rounds:")
print(f"  a: {a:08x}")
print(f"  b: {b:08x}")
print(f"  c: {c:08x}")
print(f"  d: {d:08x}")
print(f"  e: {e:08x}")

H[0] = (H[0] + a) & 0xFFFFFFFF
H[1] = (H[1] + b) & 0xFFFFFFFF
H[2] = (H[2] + c) & 0xFFFFFFFF
H[3] = (H[3] + d) & 0xFFFFFFFF
H[4] = (H[4] + e) & 0xFFFFFFFF

print("\nFinal H:")
for i, h in enumerate(H):
    print(f"  H[{i}]: {h:08x}")

# 转换为 bytes
hash_bytes = []
for h in H:
    hash_bytes.append((h >> 24) & 0xFF)
    hash_bytes.append((h >> 16) & 0xFF)
    hash_bytes.append((h >> 8) & 0xFF)
    hash_bytes.append(h & 0xFF)

hash_hex = ''.join(f'{b:02x}' for b in hash_bytes)
print(f"\nSHA-1 hash: {hash_hex}")
print(f"Expected:   a9993e364706816aba3e25717850c26c9cd0d89d")
print(f"Match: {hash_hex == 'a9993e364706816aba3e25717850c26c9cd0d89d'}")
