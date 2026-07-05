"""
上传测试数据到华为云 OBS
- 上传测试图片 test.png
- 创建并上传测试用 record.csv
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

# ========== 签名算法 (华为云 OBS V2 签名) ==========
def compute_signature(ak, sk, method, date, canonicalized_resource, content_md5="", content_type=""):
    """
    计算华为云 OBS V2 签名
    签名格式: Signature = Base64(HMAC-SHA1(SK, StringToSign))
    StringToSign = HTTP-Verb + "\n" + Content-MD5 + "\n" + Content-Type + "\n" + Date + "\n" + CanonicalizedResource
    """
    # 构造待签名字符串
    string_to_sign = f"{method}\n{content_md5}\n{content_type}\n{date}\n{canonicalized_resource}"

    print(f"\n[Signature] String to sign:")
    print(f"  {repr(string_to_sign)}")

    # 计算 HMAC-SHA1
    signature = base64.b64encode(
        hmac.new(sk.encode('utf-8'), string_to_sign.encode('utf-8'), hashlib.sha1).digest()
    ).decode('utf-8')

    print(f"[OK] Signature: {signature}")
    return signature

# ========== 上传对象 ==========
def upload_object(object_key, file_path, content_type="image/png"):
    """
    上传文件到 OBS
    """
    # 读取文件内容
    with open(file_path, 'rb') as f:
        file_data = f.read()

    # 计算 Content-MD5
    content_md5 = base64.b64encode(hashlib.md5(file_data).digest()).decode('utf-8')

    # 构造请求
    date = datetime.now(timezone.utc).strftime('%a, %d %b %Y %H:%M:%S GMT')
    canonicalized_resource = f"/{BUCKET_NAME}/{object_key}"

    # 计算签名
    signature = compute_signature(
        ACCESS_KEY_ID,
        SECRET_ACCESS_KEY,
        "PUT",
        date,
        canonicalized_resource,
        content_md5,
        content_type
    )

    # 构造请求头
    headers = {
        'Host': f"{BUCKET_NAME}.{ENDPOINT}",
        'Date': date,
        'Authorization': f"OBS {ACCESS_KEY_ID}:{signature}",
        'Content-MD5': content_md5,
        'Content-Type': content_type,
        'Content-Length': str(len(file_data))
    }

    # 发送请求
    url = f"https://{BUCKET_NAME}.{ENDPOINT}/{object_key}"
    print(f"\n[Upload] File: {file_path} -> {url}")
    print(f"[Info] File size: {len(file_data)} bytes")

    response = requests.put(url, headers=headers, data=file_data)

    print(f"\n[Response] Status code: {response.status_code}")
    print(f"[Response] Headers: {dict(response.headers)}")

    if response.status_code == 200:
        print(f"[OK] Upload success: {object_key}")
        return True
    else:
        print(f"[Error] Upload failed: {response.text}")
        return False

# ========== 创建测试 CSV ==========
def create_test_csv():
    """
    创建测试用 record.csv
    格式: Status,GarbageType,Confidence,ImageUrl,ObjectName,Timestamp,Time
    """
    csv_content = """Status,GarbageType,Confidence,ImageUrl,ObjectName,Timestamp,Time
识别完成,Recyclable,0.85,https://iotda-obs-data.obs.cn-east-3.myhuaweicloud.com/Images/garbage_001.png,Images/garbage_001.png,27100,20260425T125427Z
识别完成,Harmful,0.75,https://iotda-obs-data.obs.cn-east-3.myhuaweicloud.com/Images/garbage_002.png,Images/garbage_002.png,42295,20260425T125441Z
识别完成,Kitchen,0.92,https://iotda-obs-data.obs.cn-east-3.myhuaweicloud.com/Images/garbage_003.png,Images/garbage_003.png,57310,20260425T125502Z
识别完成,Other,0.88,https://iotda-obs-data.obs.cn-east-3.myhuaweicloud.com/Images/garbage_004.png,Images/garbage_004.png,62485,20260425T125519Z
"""
    return csv_content

# ========== 上传 CSV ==========
def upload_csv():
    """
    上传 record.csv 到 OBS
    """
    csv_content = create_test_csv()
    csv_data = csv_content.encode('utf-8')

    # 计算 Content-MD5
    content_md5 = base64.b64encode(hashlib.md5(csv_data).digest()).decode('utf-8')

    # 构造请求
    date = datetime.now(timezone.utc).strftime('%a, %d %b %Y %H:%M:%S GMT')
    object_key = "Images/records.csv"
    canonicalized_resource = f"/{BUCKET_NAME}/{object_key}"

    # 计算签名
    signature = compute_signature(
        ACCESS_KEY_ID,
        SECRET_ACCESS_KEY,
        "PUT",
        date,
        canonicalized_resource,
        content_md5,
        "text/csv"
    )

    # 构造请求头
    headers = {
        'Host': f"{BUCKET_NAME}.{ENDPOINT}",
        'Date': date,
        'Authorization': f"OBS {ACCESS_KEY_ID}:{signature}",
        'Content-MD5': content_md5,
        'Content-Type': 'text/csv',
        'Content-Length': str(len(csv_data))
    }

    # 发送请求
    url = f"https://{BUCKET_NAME}.{ENDPOINT}/{object_key}"
    print(f"\n[Upload] CSV: {url}")

    response = requests.put(url, headers=headers, data=csv_data)

    print(f"\n[Response] Status code: {response.status_code}")

    if response.status_code == 200:
        print(f"[OK] CSV upload success")
        return True
    else:
        print(f"[Error] CSV upload failed: {response.text}")
        return False

# ========== 主函数 ==========
if __name__ == "__main__":
    print("[Start] Uploading test data to OBS")

    # 1. 上传测试图片（多张，对应 CSV 中的记录）
    test_images = [
        ("Images/garbage_001.png", "reference/test.png"),
        ("Images/garbage_002.png", "reference/test.png"),
        ("Images/garbage_003.png", "reference/test.png"),
        ("Images/garbage_004.png", "reference/test.png"),
    ]

    success_count = 0
    for object_key, file_path in test_images:
        if os.path.exists(file_path):
            if upload_object(object_key, file_path):
                success_count += 1
        else:
            print(f"[Warning] File not found: {file_path}")

    print(f"\n[Summary] Images uploaded: {success_count}/{len(test_images)}")

    # 2. 上传 CSV
    if upload_csv():
        print("\n[OK] All test data uploaded successfully")
    else:
        print("\n[Error] CSV upload failed")
