#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>

/* ------------------------------
   Hàm kiểm tra xem chuỗi có phải là địa chỉ IP hợp lệ hay không
   ------------------------------ */
int is_valid_ip(const char *str) {
    int num_dots = 0;
    const char *p = str;

    // Đếm số dấu chấm '.'
    while (*p) {
        if (*p == '.') num_dots++;
        p++;
    }

    if (num_dots != 3) return 0; // IP IPv4 phải có 3 dấu '.'

    char temp[32];
    strcpy(temp, str);

    // Tách chuỗi theo dấu '.'
    char *part = strtok(temp, ".");
    int part_count = 0;

    while (part != NULL) {
        // Mỗi phần phải toàn chữ số
        for (const char *q = part; *q; q++) {
            if (!isdigit(*q)) return 0;
        }

        int num = atoi(part);
        if (num < 0 || num > 255) return 0;

        part_count++;
        part = strtok(NULL, ".");
    }

    return part_count == 4;
}

/* ------------------------------
   Hàm kiểm tra định dạng IP sai (ví dụ 1.2.3)
   ------------------------------ */
int looks_like_malformed_ip(const char *str) {
    int has_dot = 0;
    for (const char *p = str; *p; p++) {
        if (*p == '.') has_dot = 1;
        else if (!isdigit(*p)) return 0;
    }
    return has_dot;
}

/* ------------------------------
   HÀM CHÍNH
   ------------------------------ */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <domain or ip>\n", argv[0]);
        return 1;
    }

    char *param = argv[1];

    /* ------------------------------
       Nếu là địa chỉ IP hợp lệ
       ------------------------------ */
    if (is_valid_ip(param)) {
        struct in_addr addr;

        // Chuyển IP text → nhị phân
        if (inet_aton(param, &addr) == 0) {
            printf("Invalid address\n");
            return 1;
        }

        // Tra ngược lại tên miền
        struct hostent *host = gethostbyaddr(&addr, sizeof(addr), AF_INET);
        if (!host) {
            printf("Not found information\n");
            return 0;
        }

        printf("Official name: %s", host->h_name);

        // In các alias name
        int first_alias = 1;
        for (char **a = host->h_aliases; *a; a++) {
            if (first_alias) {
                printf(" Alias name: %s", *a);
                first_alias = 0;
            } else {
                printf(" %s", *a);
            }
        }
        printf("\n");
    }

    /* ------------------------------
       Nếu giống IP nhưng sai định dạng
       ------------------------------ */
    else if (looks_like_malformed_ip(param)) {
        printf("Invalid address\n");
    }

    /* ------------------------------
       Nếu là tên miền
       ------------------------------ */
    else {
        struct hostent *host = gethostbyname(param);
        if (!host) {
            printf("Not found information\n");
            return 0;
        }

        // IP chính thức (đầu tiên)
        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, host->h_addr_list[0], ipbuf, INET_ADDRSTRLEN);
        printf("Official IP: %s", ipbuf);

        // Các IP alias (phụ)
        int first_alias = 1;
        for (char **ptr = host->h_addr_list + 1; *ptr; ptr++) {
            inet_ntop(AF_INET, *ptr, ipbuf, INET_ADDRSTRLEN);
            if (first_alias) {
                printf(" Alias IP: %s", ipbuf);
                first_alias = 0;
            } else {
                printf(" %s", ipbuf);
            }
        }
        printf("\n");
    }

    return 0;
}
