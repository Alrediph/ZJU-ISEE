#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
验证 OBS 中实际存储的图片文件名
检查 Images/ 目录下的对象列表
"""

import hashlib
import hmac
import base64
import requests
from datetime import datetime, timezone
from xml.etree import ElementTree

# ========== 配置信息 ==========
ACCESS_KEY_ID = "HPUAW2GYOGAJ2OGCYLLP"
SECRET_ACCESS_KEY = "aFEoJpm8QvYlXOfi6w3WXCLAoIoZav2ClfVIjiwE"
ENDPOINT = "obs.cn-east-3.myhuaweicloud.com"
BUCKET_NAME = "iotda-obs-data"

# ========== 签名算法 ==========
def compute_signature(ak, sk, method, date, canonicalized_resource, content_md5="", content_type=""):
    """计算华为云 OBS V2 签名"""
    string_to_sign = f"{method}\n{content_md5}\n{content_type}\n{date}\n{canonicalized_resource}"
    signature = base64.b64encode(
        hmac.new(sk.encode('utf-8'), string_to_sign.encode('utf-8'), hashlib.sha1).digest()
    ).decode('utf-8')
    return signature

# ========== 列出对象 ==========
def list_objects(prefix="Images/"):
    """列出 OBS 桶中指定前缀的对象"""
    date = datetime.now(timezone.utc).strftime('%a, %d %b %Y %H:%M:%S GMT')
    canonicalized_resource = f"/{BUCKET_NAME}/?prefix={prefix}"

    # 计算签名
    signature = compute_signature(
        ACCESS_KEY_ID,
        SECRET_ACCESS_KEY,
        "GET",
        date,
        canonicalized_resource
    )

    # 构造请求头
    headers = {
        'Host': f"{BUCKET_NAME}.{ENDPOINT}",
        'Date': date,
        'Authorization': f"OBS {ACCESS_KEY_ID}:{signature}"
    }

    # 发送请求
    url = f"https://{BUCKET_NAME}.{ENDPOINT}/?prefix={prefix}"
    print(f"[List] URL: {url}\n")

    response = requests.get(url, headers=headers)

    print(f"[Response] Status code: {response.status_code}")

    if response.status_code == 200:
        # 解析 XML 响应
        root = ElementTree.fromstring(response.content)

        # 提取对象信息
        contents = root.findall('.//{http://obs.cn-east-3.myhuaweicloud.com/doc/2016-01-01/}Contents')

        print(f"\n[Objects in '{prefix}']:")
        print("-" * 70)

        if not contents:
            print("  (no objects found)")
        else:
            for content in contents:
                key = content.find('{http://obs.cn-east-3.myhuaweicloud.com/doc/2016-01-01/}Key').text
                size = content.find('{http://obs.cn-east-3.myhuaweicloud.com/doc/2016-01-01/}Size').text
                print(f"  Key: {key}")
                print(f"  Size: {size} bytes")
                print()

        return [c.find('{http://obs.cn-east-3.myhuaweicloud.com/doc/2016-01-01/}Key').text for c in contents]
    else:
        print(f"[Error] List failed: {response.text}")
        return []

# ========== 检查单个对象是否存在 ==========
def check_object_exists(object_key):
    """检查指定对象是否存在（HEAD 请求）"""
    date = datetime.now(timezone.utc).strftime('%a, %d %b %Y %H:%M:%S GMT')
    canonicalized_resource = f"/{BUCKET_NAME}/{object_key}"

    # 计算签名
    signature = compute_signature(
        ACCESS_KEY_ID,
        SECRET_ACCESS_KEY,
        "HEAD",
        date,
        canonicalized_resource
    )

    # 构造请求头
    headers = {
        'Host': f"{BUCKET_NAME}.{ENDPOINT}",
        'Date': date,
        'Authorization': f"OBS {ACCESS_KEY_ID}:{signature}"
    }

    # 发送请求
    url = f"https://{BUCKET_NAME}.{ENDPOINT}/{object_key}"
    response = requests.head(url, headers=headers)

    exists = response.status_code == 200
    print(f"[Check] {object_key}: {'EXISTS' if exists else 'NOT FOUND'} (status: {response.status_code})")

    return exists

# ========== 主函数 ==========
if __name__ == "__main__":
    print("=" * 70)
    print("OBS 对象验证")
    print("=" * 70)

    # 1. 列出 Images/ 目录下的所有对象
    print("\n[Step 1] Listing objects in Images/ directory:")
    objects = list_objects("Images/")

    # 2. 检查 CSV 中引用的图片文件
    print("\n[Step 2] Checking images referenced in CSV:")
    csv_images = [
        "Images/garbage_001.png",
        "Images/garbage_002.png",
        "Images/garbage_003.png",
        "Images/garbage_004.png"
    ]

    for img in csv_images:
        check_object_exists(img)

    # 3. 检查错误 URL 中的图片名称
    print("\n[Step 3] Checking problematic image name from error:")
    # 错误 URL 中的图片名称：其他垃圾（URL 编码后）
    problem_image = "Images/其他垃圾"  # 这是从错误 URL 解码得到的
    check_object_exists(problem_image)

    print("\n" + "=" * 70)
    print("验证完成")
    print("=" * 70)
