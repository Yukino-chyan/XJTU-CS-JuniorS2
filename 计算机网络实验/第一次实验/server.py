import socket

def start_server():
    host = '0.0.0.0'
    port = 9999
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((host, port))
    server_socket.listen(5)
    print("=" * 50)
    print(f"[系统提示] TCP 服务器已启动！监听端口: {port}")
    print("=" * 50 + "\n")
    while True:
        conn, addr = server_socket.accept()
        print(f"[新连接接入] 客户端 {addr} 已连接。")
        client_ip = addr[0]
        
        try:
            while True:
                data = conn.recv(1024)
                if not data:
                    print(f"[连接关闭] 客户端 {addr} 正常退出。")
                    break 
                original_msg = data.decode('utf-8')
                print(f"  >>> 收到消息 | from {addr}: {original_msg}")
                modified_msg = f"{original_msg},{client_ip}!"
                conn.sendall(modified_msg.encode('utf-8'))
                
        except ConnectionResetError:
            pass
        finally:
            conn.close()
            print("-" * 50)

if __name__ == '__main__':
    start_server()