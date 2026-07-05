#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
列出 OBS 桶中 Images 目录下的所有对象
"""

import hashlib
import hmac
import base64
import requests
from datetime import datetime, timezone
import xml.etree.ElementTree as ET

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

# ========== 列出桶内对象 ==========
def list_objects(prefix=""):
    """列出指定前缀的对象"""
    date = datetime.now(timezone.utc).strftime('%a, %d %b %Y %H:%M:%S GMT')

    # 对于列表操作，canonicalized_resource 只需要桶名
    canonicalized_resource = f"/{BUCKET_NAME}/"

    signature = compute_signature(
        ACCESS_KEY_ID, SECRET_ACCESS_KEY,
        "GET", date, canonicalized_resource
    )

    headers = {
        'Host': f"{BUCKET_NAME}.{ENDPOINT}",
        'Date': date,
        'Authorization': f"OBS {ACCESS_KEY_ID}:{signature}"
    }

    # 构造 URL（查询参数不需要在签名中）
    if prefix:
        url = f"https://{BUCKET_NAME}.{ENDPOINT}/?prefix={prefix}&max-keys=100"
    else:
        url = f"https://{BUCKET_NAME}.{ENDPOINT}/?max-keys=100"

    response = requests.get(url, headers=headers)

    if response.status_code == 200:
        # 解析 XML
        root = ET.fromstring(response.content)

        # 提取对象列表
        objects = []
        for contents in root.findall('{http://obs.cn-east-3.myhuaweicloud.com/doc/2015-06-30/}Contents'):
            key = contents.find('{http://obs.cn-east-3.myhuaweicloud.com/doc/2015-06-30/}Key').text
            size = contents.find('{http://obs.cn-east-3.myhuaweicloud.com/doc/2015-06-30/}Size').text
            last_modified = contents.find('{http://obs.cn-east-3.myhuaweicloud.com/doc/2015-06-30/}LastModified').text
            objects.append({
                'key': key,
                'size': int(size),
                'last_modified': last_modified
            })

        return objects
    else:
        print(f"[Error] List objects failed: {response.status_code}")
        print(response.text)
        return []

# ========== 主函数 ==========
if __name__ == "__main__":
    print("=" * 70)
    print("OBS Images 目录对象列表")
    print("=" * 70)
    print()

    objects = list_objects("")

    if objects:
        print(f"[Found] {len(objects)} objects:")
        for obj in objects:
            print(f"  - {obj['key']} ({obj['size']} bytes, {obj['last_modified']})")
    else:
        print("[Empty] No objects found in bucket")

    print()

    # 检查是否有 jpg 文件
    jpg_files = [obj for obj in objects if obj['key'].endswith('.jpg')]
    png_files = [obj for obj in objects if obj['key'].endswith('.png')]
    csv_files = [obj for obj in objects if obj['key'].endswith('.csv')]

    print(f"[Summary]")
    print(f"  JPG files: {len(jpg_files)}")
    print(f"  PNG files: {len(png_files)}")
    print(f"  CSV files: {len(csv_files)}")