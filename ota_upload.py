#!/usr/bin/env python3
"""
OTA Push Upload Script cho ESP32 IR Controller.

Cách dùng:
  python ota_upload.py                         # Dùng IP mặc định
  python ota_upload.py --host 192.168.1.100    # Chỉ định IP
  python ota_upload.py --bin path/to/firmware.bin  # Chỉ định file firmware

Luồng:
  1. Đọc firmware .bin, tính MD5
  2. POST /api/ota/begin  → ESP32 chuẩn bị flash
  3. POST /api/ota/upload → Gửi firmware binary
  4. ESP32 tự restart với firmware mới
"""

import argparse
import hashlib
import os
import sys
import time

try:
    import requests
except ImportError:
    print("ERROR: Cần cài thư viện 'requests'.")
    print("       pip install requests")
    sys.exit(1)

# ---------- defaults ----------
DEFAULT_HOST = "192.168.110.192"       # IP ESP32 từ Serial Monitor
DEFAULT_PORT = 80
DEFAULT_BIN  = ".pio/build/esp32doit-devkit-v1/firmware.bin"
TIMEOUT      = 30  # seconds


def md5_of_file(path: str) -> str:
    h = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            h.update(chunk)
    return h.hexdigest()


def ota_upload(host: str, port: int, bin_path: str):
    base_url = f"http://{host}:{port}"

    # --- Kiểm tra file ---
    if not os.path.isfile(bin_path):
        print(f"[ERROR] Không tìm thấy firmware: {bin_path}")
        sys.exit(1)

    fw_size = os.path.getsize(bin_path)
    fw_md5  = md5_of_file(bin_path)

    print(f"============================================")
    print(f"          ESP32 OTA Push Upload             ")
    print(f"============================================")
    print(f"  Host : {host}:{port}")
    print(f"  File : {bin_path}")
    print(f"  Size : {fw_size:,} bytes")
    print(f"  MD5  : {fw_md5}")
    print(f"============================================")

    # --- Bước 1: POST /api/ota/begin ---
    print("\n[1/2] Gui /api/ota/begin ...")
    try:
        r = requests.post(
            f"{base_url}/api/ota/begin",
            data={"size": str(fw_size), "md5": fw_md5},
            timeout=TIMEOUT,
        )
    except requests.ConnectionError:
        print(f"[ERROR] Khong ket noi duoc toi {base_url}")
        print(f"        Kiem tra ESP32 da ket noi WiFi va IP dung chua.")
        sys.exit(1)

    if r.status_code != 200:
        print(f"[ERROR] ESP32 tra loi: {r.status_code} — {r.text}")
        sys.exit(1)

    resp = r.json()
    if not resp.get("ready"):
        print(f"[ERROR] ESP32 chua san sang: {resp}")
        sys.exit(1)

    print("       -> ESP32 san sang nhan firmware (OK)")

    # --- Bước 2: POST /api/ota/upload ---
    print("[2/2] Dang truyen firmware ...")
    start = time.time()

    with open(bin_path, "rb") as f:
        r = requests.post(
            f"{base_url}/api/ota/upload",
            files={"firmware": ("firmware.bin", f, "application/octet-stream")},
            timeout=120,
        )

    elapsed = time.time() - start

    if r.status_code == 200:
        resp = r.json()
        print(f"\n============================================")
        print(f"  [OK] OTA THANH CONG! ({elapsed:.1f}s)")
        print(f"  -> {resp.get('msg', '')}")
        print(f"  ESP32 dang restart...")
        print(f"============================================")
    else:
        print(f"\n[ERROR] Upload that bai: {r.status_code} — {r.text}")
        sys.exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ESP32 OTA Push Upload")
    parser.add_argument("--host", default=DEFAULT_HOST,
                        help=f"IP của ESP32 (mặc định: {DEFAULT_HOST})")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT,
                        help=f"Port (mặc định: {DEFAULT_PORT})")
    parser.add_argument("--bin", default=DEFAULT_BIN,
                        help=f"Đường dẫn firmware .bin (mặc định: {DEFAULT_BIN})")
    args = parser.parse_args()

    ota_upload(args.host, args.port, args.bin)
