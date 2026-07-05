#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
上传实际的 records.csv 到 OBS（替换测试数据）
并检查实例图片是否存在
"""

import hashlib
import hmac
import base64
import requests
from datetime import datetime, timezone
import os

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

# ========== 检查对象是否存在 ==========
def check_object_exists(object_key):
    """检查 OBS 对象是否存在（HEAD 请求）"""
    date = datetime.now(timezone.utc).strftime('%a, %d %b %Y %H:%M:%S GMT')
    canonicalized_resource = f"/{BUCKET_NAME}/{object_key}"

    signature = compute_signature(
        ACCESS_KEY_ID, SECRET_ACCESS_KEY,
        "GET", date, canonicalized_resource
    )

    headers = {
        'Host': f"{BUCKET_NAME}.{ENDPOINT}",
        'Date': date,
        'Authorization': f"OBS {ACCESS_KEY_ID}:{signature}"
    }

    url = f"https://{BUCKET_NAME}.{ENDPOINT}/{object_key}"
    response = requests.head(url, headers=headers)

    return response.status_code == 200

# ========== 上传 CSV ==========
def upload_csv(csv_content):
    """上传 CSV 到 OBS"""
    csv_data = csv_content.encode('utf-8')
    content_md5 = base64.b64encode(hashlib.md5(csv_data).digest()).decode('utf-8')

    date = datetime.now(timezone.utc).strftime('%a, %d %b %Y %H:%M:%S GMT')
    object_key = "Images/records.csv"
    canonicalized_resource = f"/{BUCKET_NAME}/{object_key}"

    signature = compute_signature(
        ACCESS_KEY_ID, SECRET_ACCESS_KEY,
        "PUT", date, canonicalized_resource,
        content_md5, "text/csv"
    )

    headers = {
        'Host': f"{BUCKET_NAME}.{ENDPOINT}",
        'Date': date,
        'Authorization': f"OBS {ACCESS_KEY_ID}:{signature}",
        'Content-MD5': content_md5,
        'Content-Type': 'text/csv',
        'Content-Length': str(len(csv_data))
    }

    url = f"https://{BUCKET_NAME}.{ENDPOINT}/{object_key}"
    print(f"[Upload] CSV to: {url}")

    response = requests.put(url, headers=headers, data=csv_data)

    if response.status_code == 200:
        print(f"[OK] CSV upload success")
        return True
    else:
        print(f"[Error] CSV upload failed: {response.status_code} - {response.text}")
        return False

# ========== 主函数 ==========
if __name__ == "__main__":
    print("=" * 70)
    print("上传实际 records.csv 并检查实例图片")
    print("=" * 70)
    print()

    # 1. 读取实际的 records.csv
    csv_file = "reference/records.csv"
    if os.path.exists(csv_file):
        with open(csv_file, 'r', encoding='utf-8') as f:
            csv_content = f.read()
        print(f"[Read] {csv_file}")
        print(f"[Content]:")
        print(csv_content)
        print()
    else:
        print(f"[Error] File not found: {csv_file}")
        exit(1)

    # 2. 解析 CSV 获取图片列表
    lines = csv_content.strip().split('\n')
    image_keys = []
    if lines and 'Status' in lines[0]:
        lines = lines[1:]

    for line in lines:
        parts = line.split(',')
        if len(parts) >= 5:
            object_name = parts[4].strip()  # ObjectName 字段
            image_keys.append(object_name)

    print(f"[Images] Found {len(image_keys)} images in CSV:")
    for img in image_keys:
        print(f"  - {img}")
    print()

    # 3. 检查图片是否存在
    print("[Check] Verifying images in OBS...")
    for img_key in image_keys:
        exists = check_object_exists(img_key)
        status = "✅ EXISTS" if exists else "❌ NOT FOUND"
        print(f"  {status}: {img_key}")
    print()

    # 4. 上传 CSV
    print("[Upload] Uploading records.csv to OBS...")
    if upload_csv(csv_content):
        print("\n[OK] All done!")
    else:
        print("\n[Error] Upload failed")
