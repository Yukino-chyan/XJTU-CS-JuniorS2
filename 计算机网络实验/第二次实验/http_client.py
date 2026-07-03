import socket
import sys
import time

def main():
    if len(sys.argv) < 3:
        print("用法: python http_client.py <主机名或IP> <文件路径>")
        print("示例: python http_client.py 114.215.169.231 /m25.4.zip")
        sys.exit(1)

    hostname = sys.argv[1]
    filepath = sys.argv[2]       # 新增：要下载的文件路径
    port = 80

    # DNS解析
    start_dns = time.perf_counter()
    ip_address = socket.gethostbyname(hostname)
    end_dns = time.perf_counter()
    time_dns = (end_dns - start_dns) * 1000

    # TCP连接
    start_tcp = time.perf_counter()
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.settimeout(30)
    client_socket.connect((ip_address, port))
    end_tcp = time.perf_counter()
    time_tcp = (end_tcp - start_tcp) * 1000
    print(f"[*] TCP连接成功 (端口 80)")

    # 构造HTTP请求 —— 用filepath替代写死的"/"
    request = (
        f"GET {filepath} HTTP/1.1\r\n"
        f"Host: {hostname}\r\n"
        f"User-Agent: You Haochen\r\n"
        f"Connection: close\r\n\r\n"
    )

    print(f"[*] 正在下载 {filepath} ...\n")

    # 发送请求并接收响应
    start_http = time.perf_counter()
    response_data = b""
    try:
        client_socket.sendall(request.encode('utf-8'))
        while True:
            recv_data = client_socket.recv(65536)  # 加大缓冲区，下载大文件更快
            if not recv_data:
                break
            response_data += recv_data
    except Exception as e:
        print(f"[!] 数据传输过程发生异常: {e}")
    finally:
        client_socket.close()
    end_http = time.perf_counter()
    time_http = (end_http - start_http) * 1000

    # 分离HTTP头部和文件内容
    header_end = response_data.find(b"\r\n\r\n")
    if header_end == -1:
        print("[!] 未找到HTTP响应头部")
        return

    header = response_data[:header_end].decode('utf-8', errors='ignore')
    body = response_data[header_end + 4:]

    # 打印响应头
    print("-" * 50)
    print(header)
    print("-" * 50)

    # 保存文件到本地（取文件路径最后一段作为文件名）
    filename = filepath.split("/")[-1]
    if filename:
        with open(filename, "wb") as f:
            f.write(body)
        print(f"\n[*] 文件已保存: {filename}")
        print(f"[*] 文件大小: {len(body)} 字节 ({len(body)/1024/1024:.2f} MB)")
    else:
        print("\n[*] 响应内容(前500字符):")
        print(body[:500].decode('utf-8', errors='ignore'))

    # 时间统计报告（指导书步骤3.1要求）
    print("\n" + "=" * 40)
    print("【连接与传输时间统计报告】")
    print(f"文件名:              {filename}")
    print(f"1. DNS解析耗时:      {time_dns:.2f} ms")
    print(f"2. TCP建立连接耗时:  {time_tcp:.2f} ms")
    print(f"3. 下载总耗时:       {time_http:.2f} ms")
    print("=" * 40 + "\n")

if __name__ == "__main__":
    main()