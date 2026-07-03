import socket
import sys
import os

def main():
    host = "0.0.0.0"
    port = 9000
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((host, port))
    server_socket.listen(5)
    print(f"[*] 文件传输服务器启动，监听端口 {port}")
    server_private_ip = socket.gethostbyname(socket.gethostname())
    print(f"[*] 云服务器私网地址: {server_private_ip}")

    while True:
        conn, addr = server_socket.accept()
        print(f"[*] 客户端公网地址: {addr[0]}:{addr[1]}")
        print(f"[*] 云服务器私网地址: {server_private_ip}")
        try:
            filename = b""
            while True:
                byte = conn.recv(1)
                if byte == b"\n" or not byte:
                    break
                filename += byte
            filename = filename.decode("utf-8").strip()
            print(f"[*] 请求文件: {filename}")

            if not os.path.isfile(filename):
                conn.sendall(b"ERROR: File not found\n")
                print(f"[!] 文件不存在: {filename}")
                conn.close()
                continue

            filesize = os.path.getsize(filename)
            conn.sendall(f"{filesize}\n".encode("utf-8"))

            sent = 0
            with open(filename, "rb") as f:
                while True:
                    data = f.read(4096)
                    if not data:
                        break
                    conn.sendall(data)
                    sent += len(data)

            print(f"[*] 发送完成: {filename} ({sent} 字节)")

        except Exception as e:
            print(f"[!] 异常: {e}")
        finally:
            conn.close()

if __name__ == "__main__":
    main()