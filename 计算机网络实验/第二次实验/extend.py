import socket
import sys
import time
import re

def fetch_object(hostname, path):
    t0 = time.perf_counter()
    ip = socket.gethostbyname(hostname)
    t_dns = (time.perf_counter() - t0) * 1000
    t1 = time.perf_counter()
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(5)
    s.connect((ip, 80))
    t_tcp = (time.perf_counter() - t1) * 1000
    req = f"GET {path} HTTP/1.1\r\nHost: {hostname}\r\nUser-Agent: You Haochen\r\nConnection: close\r\n\r\n"
    t2 = time.perf_counter()
    s.sendall(req.encode('utf-8'))
    resp = b""
    while True:
        try:
            d = s.recv(4096)
            if not d: break
            resp += d
        except:
            break
    s.close()
    t_http = (time.perf_counter() - t2) * 1000
    return resp, t_dns, t_tcp, t_http

def main():
    if len(sys.argv) < 2:
        sys.exit(1)    
    host = sys.argv[1].replace("http://", "").split("/")[0]
    print(f"Target: {host}")
    resp, total_dns, total_tcp, total_http = fetch_object(host, "/")
    objs_count = 1
    html = resp.decode('utf-8', errors='ignore')
    urls = re.findall(r'(?:src|href)=["\']([^"\']+\.(?:jpg|jpeg|png|gif|css|js|ico))["\']', html)
    fetched = set()
    for url in urls:
        if url in fetched or url.startswith("https://") or url.startswith("data:"): 
            continue
        fetched.add(url)
        t_host, t_path = host, url
        if url.startswith("http://"):
            parts = url[7:].split("/", 1)
            t_host, t_path = parts[0], "/" + (parts[1] if len(parts) > 1 else "")
        elif url.startswith("//"):
            parts = url[2:].split("/", 1)
            t_host, t_path = parts[0], "/" + (parts[1] if len(parts) > 1 else "")
        elif not url.startswith("/"):
            t_path = "/" + url
        try:
            _, d, t, h = fetch_object(t_host, t_path)
            total_dns += d
            total_tcp += t
            total_http += h
            objs_count += 1
        except:
            continue
    print(f"Total Objects Fetched: {objs_count}")
    print(f"Total DNS Time:        {total_dns:.2f} ms")
    print(f"Total TCP Time:        {total_tcp:.2f} ms")
    print(f"Total HTTP Time:       {total_http:.2f} ms")

if __name__ == "__main__":
    main()