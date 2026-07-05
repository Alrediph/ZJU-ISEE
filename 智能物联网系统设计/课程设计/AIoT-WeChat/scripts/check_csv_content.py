#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
下载并检查 OBS 中的 record.csv 内容
"""

import hashlib
import hmac
import base64
import requests
from datetime import datetime, timezone

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

# ========== 下载 CSV ==========
def download_csv():
    """下载 record.csv"""
    date = datetime.now(timezone.utc).strftime('%a, %d %b %Y %H:%M:%S GMT')
    object_key = "Images/records.csv"
    canonicalized_resource = f"/{BUCKET_NAME}/{object_key}"

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
    url = f"https://{BUCKET_NAME}.{ENDPOINT}/{object_key}"
    print(f"[Download] URL: {url}\n")

    response = requests.get(url, headers=headers)

    print(f"[Response] Status code: {response.status_code}")

    if response.status_code == 200:
        csv_content = response.text
        print(f"\n[CSV Content]:")
        print("=" * 70)
        print(csv_content)
        print("=" * 70)

        # 解析 CSV
        print(f"\n[CSV Parsing]:")
        lines = csv_content.strip().split('\n')
        print(f"Total lines: {len(lines)}")

        # 跳过 header
        if lines and 'Status' in lines[0]:
            lines = lines[1:]

        for i, line in enumerate(lines):
            parts = line.split(',')
            if len(parts) >= 7:
                status, garbage_type, confidence, image_url, object_name, timestamp, time_val = parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6]
                print(f"\n  Record {i+1}:")
                print(f"    status: {status}")
                print(f"    garbage_type: {garbage_type}")
                print(f"    confidence: {confidence}")
                print(f"    image_url: {image_url}")
                print(f"    object_name: {object_name}")
                print(f"    timestamp: {timestamp}")
                print(f"    time: {time_val}")

        return csv_content
    else:
        print(f"[Error] Download failed: {response.text}")
        return None

# ========== 主函数 ==========
if __name__ == "__main__":
    print("=" * 70)
    print("OBS record.csv 内容检查")
    print("=" * 70)
    print()

    download_csv()
