

---

# Thực hành Lập trình Mạng 

## Giới thiệu

Bộ bài thực hành này gồm **2 phần** nhằm giúp làm quen với các hàm thư viện mạng trong C (socket, `gethostbyname`, `gethostbyaddr`, `inet_ntoa`, `inet_pton`, ...).  
Sẽ học cách:
- Kiểm tra tính hợp lệ của địa chỉ IP.
- Phân biệt địa chỉ đặc biệt (loopback, private, multicast...).
- Tra cứu tên miền → địa chỉ IP.
- Tra cứu địa chỉ IP → tên miền (reverse DNS lookup).




---

## ▶️ Cách chạy chương trình

### 1️⃣ Tra cứu tên miền

```bash
$ ./resolver google.com
Official name: google.com
Alias name: www.google.com
IP address: 142.250.199.46
```

### 2️⃣ Tra cứu địa chỉ IP

```bash
$ ./resolver 8.8.8.8
Official name: dns.google
Alias name: 8.8.8.8.in-addr.arpa
```

---

## 🧱 Giải thích mã nguồn

### 1. Hàm kiểm tra địa chỉ IP hợp lệ

```c
int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr));
}
```

→ Dùng `inet_pton` để xác định chuỗi nhập vào có phải IPv4 hợp lệ hay không.

---

### 2. Hàm tra cứu tên miền

```c
struct hostent *host = gethostbyname(domain);
```

→ Trả về cấu trúc chứa danh sách các địa chỉ IP của tên miền.

---

### 3. Hàm tra cứu IP (reverse lookup)

```c
struct hostent *host = gethostbyaddr(&addr, sizeof(addr), AF_INET);
```

→ Trả về tên miền chính thức (official name) tương ứng với địa chỉ IP.

---

### 4. Xử lý lỗi "Invalid address"

Nguyên nhân: nhập chuỗi tên miền chưa được chuẩn hóa hoặc không tồn tại.
Cách khắc phục:

* Kiểm tra định dạng trước khi gọi `inet_pton`.
* Phân nhánh hợp lý giữa `gethostbyname` (domain) và `gethostbyaddr` (IP).
* Sử dụng `herror()` để hiển thị lỗi chi tiết.

---

## Thư viện sử dụng

| Thư viện         | Mục đích                                           |
| ---------------- | -------------------------------------------------- |
| `<stdio.h>`      | Nhập/xuất                                          |
| `<stdlib.h>`     | Quản lý bộ nhớ                                     |
| `<string.h>`     | Xử lý chuỗi                                        |
| `<arpa/inet.h>`  | Chuyển đổi IP, cấu trúc địa chỉ mạng               |
| `<netdb.h>`      | Hàm tra cứu DNS (`gethostbyname`, `gethostbyaddr`) |
| `<netinet/in.h>` | Định nghĩa cấu trúc `sockaddr_in`                  |
| `<sys/socket.h>` | Các kiểu dữ liệu socket                            |

---

## Kiến thức rút ra

* Phân biệt được IP và hostname.
* Biết cách tra cứu DNS bằng hàm C chuẩn.
* Hiểu cơ chế ánh xạ **Domain ↔ IP**.
* Làm quen với xử lý lỗi mạng trong ngôn ngữ C.

---

## Gợi ý mở rộng

* Thêm tùy chọn giao diện dòng lệnh nâng cao (`getopt`).
* Hỗ trợ IPv6 (`AF_INET6`).
* Ghi log kết quả ra file text.
* Tích hợp multithreading để tra cứu song song nhiều địa chỉ.


---


