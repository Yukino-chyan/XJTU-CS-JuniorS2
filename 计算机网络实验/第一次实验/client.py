import socket

def run_client():
    
    target_ip = '127.0.0.1'
    target_port = 9999
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((target_ip, target_port))
        print("连接成功！\n")
        for i in range(1, 6):
            msg = input(f"第 {i} 行输入: ")
            client_socket.send(msg.encode('utf-8'))
            response = client_socket.recv(1024)
            print(f"服务器回复: {response.decode('utf-8')}\n")
            
    finally:
        client_socket.close()
        print("连接已关闭。")

if __name__ == '__main__':
    run_client()