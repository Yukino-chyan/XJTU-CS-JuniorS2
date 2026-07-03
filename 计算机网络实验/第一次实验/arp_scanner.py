from scapy.all import ARP, Ether, srp
import sys

def arp_scan(ip_range):
    print(f"[*] 正在扫描网段: {ip_range}\n")
    arp_request = ARP(pdst=ip_range)
    broadcast = Ether(dst="ff:ff:ff:ff:ff:ff")
    arp_request_broadcast = broadcast / arp_request
    answered_list = srp(arp_request_broadcast, timeout=2, verbose=True, iface="Intel(R) Wi-Fi 6 AX201 160MHz")[0]
    clients_list = []
    for element in answered_list:
        client_dict = {"ip": element[1].psrc, "mac": element[1].hwsrc}
        clients_list.append(client_dict)
    return clients_list

def print_result(results):
    print("IP 地址\t\t\tMAC 地址")
    print("-" * 45)
    for client in results:
        print(f"{client['ip']}\t\t{client['mac']}")

if __name__ == "__main__":
    target_network = "172.20.10.0/28"
    scan_results = arp_scan(target_network)
    print_result(scan_results)
    print(f"\n[*] 扫描完成，共发现 {len(scan_results)} 台存活主机。")