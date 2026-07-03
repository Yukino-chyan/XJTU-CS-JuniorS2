import socket
import sys
import time

def main():
    if len(sys.argv) < 3:
        print("用法: python tcp_client.py <服务器IP> <文件名>")
        print("示例: python tcp_client.py 114.215.169.231 m25.4.zip")
        sys.exit(1)
    host = sys.argv[1]
    filename = sys.argv[2]
    port = 9000
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((host, port))
    print(f"[*] 已连接到 {host}:{port}")
    client_socket.sendall((filename + "\n").encode("utf-8"))
    size_str = b""
    while True:
        byte = client_socket.recv(1)
        if byte == b"\n" or not byte:
            break
        size_str += byte
    size_str = size_str.decode("utf-8").strip()

    if size_str.startswith("ERROR"):
        print(f"[!] 服务器返回错误: {size_str}")
        client_socket.close()
        return

    filesize = int(size_str)
    print(f"[*] 文件大小: {filesize} 字节 ({filesize/1024/1024:.2f} MB)")

    start_time = time.perf_counter()
    received = 0
    with open("download_" + filename, "wb") as f:
        while received < filesize:
            data = client_socket.recv(65536)
            if not data:
                break
            f.write(data)
            received += len(data)
            pct = received / filesize * 100
            print(f"\r[*] 下载进度: {pct:.1f}%", end="", flush=True)

    elapsed = time.perf_counter() - start_time
    client_socket.close()
    print()
    print(f"[*] 下载完成: download_{filename}")
    print(f"[*] 耗时: {elapsed*1000:.2f} ms")
if __name__ == "__main__":
    main()