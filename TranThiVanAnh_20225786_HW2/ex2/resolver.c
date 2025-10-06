#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>

/* ---------------------------------
   HÀM KIỂM TRA IP HỢP LỆ (IPv4)
   --------------------------------- */
int is_valid_ipv4(const char *str) {
    int dots = 0;
    const char *p = str;
    while (*p) {
        if (*p == '.') dots++;
        else if (!isdigit(*p)) return 0;
        p++;
    }
    if (dots != 3) return 0;

    char temp[64];
    strcpy(temp, str);
    char *part = strtok(temp, ".");
    int count = 0;
    while (part) {
        int num = atoi(part);
        if (num < 0 || num > 255) return 0;
        count++;
        part = strtok(NULL, ".");
    }
    return count == 4;
}

/* ---------------------------------
   HÀM KIỂM TRA IP HỢP LỆ (IPv6)
   --------------------------------- */
int is_valid_ipv6(const char *str) {
    struct in6_addr addr6;
    return inet_pton(AF_INET6, str, &addr6) == 1;
}

/* ---------------------------------
   HÀM KIỂM TRA IP ĐẶC BIỆT
   --------------------------------- */
int is_special_ip(struct in_addr addr) {
    unsigned long ip = ntohl(addr.s_addr);
    if ((ip >= 0x0A000000 && ip <= 0x0AFFFFFF) ||     // 10.0.0.0/8
        (ip >= 0xAC100000 && ip <= 0xAC1FFFFF) ||     // 172.16.0.0/12
        (ip >= 0xC0A80000 && ip <= 0xC0A8FFFF) ||     // 192.168.0.0/16
        (ip == 0x7F000001))                           // 127.0.0.1
        return 1;
    return 0;
}

/* ---------------------------------
   HÀM GHI LOG
   --------------------------------- */
void write_log(const char *input, const char *result) {
    FILE *f = fopen("resolver_log.txt", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(f, "[%02d-%02d-%04d %02d:%02d:%02d] %s => %s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            input, result);
    fclose(f);
}

/* ---------------------------------
   HÀM TRA CỨU IP → TÊN MIỀN
   --------------------------------- */
void resolve_ip(const char *ip_str) {
    struct in_addr addr;
    char result[1024] = "";

    if (inet_aton(ip_str, &addr) == 0) {
        printf("Invalid address\n");
        write_log(ip_str, "Invalid address");
        return;
    }

    if (is_special_ip(addr)) {
        printf("⚠️  Special IP address — may not have DNS record\n");
    }

    struct hostent *host = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (!host) {
        printf("Not found information\n");
        write_log(ip_str, "Not found information");
        return;
    }

    printf("Official name: %s\n", host->h_name);
    strcat(result, host->h_name);

    if (host->h_aliases[0]) {
        printf("Alias names: ");
        for (char **a = host->h_aliases; *a; a++) {
            printf("%s ", *a);
            strcat(result, " ");
            strcat(result, *a);
        }
        printf("\n");
    }

    write_log(ip_str, result);
}

/* ---------------------------------
   HÀM TRA CỨU TÊN MIỀN → IP
   --------------------------------- */
void resolve_domain(const char *domain) {
    struct addrinfo hints, *res, *p;
    char ipstr[INET6_ADDRSTRLEN];
    char result[2048] = "";

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // hỗ trợ IPv4 và IPv6
    hints.ai_socktype = SOCK_STREAM;

    time_t start = time(NULL);
    int status = getaddrinfo(domain, NULL, &hints, &res);
    time_t end = time(NULL);

    if (status != 0) {
        printf("Not found information\n");
        write_log(domain, "Not found information");
        return;
    }

    printf("Canonical name (CNAME): %s\n", res->ai_canonname ? res->ai_canonname : "(none)");

    int first = 1;
    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        const char *ipver;

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        if (first) {
            printf("Official %s: %s\n", ipver, ipstr);
            sprintf(result, "%s %s", ipver, ipstr);
            first = 0;
        } else {
            printf("Alias %s: %s\n", ipver, ipstr);
            strcat(result, " ");
            strcat(result, ipstr);
        }
    }

    printf("Query time: %.2f seconds\n", difftime(end, start));

    write_log(domain, result);
    freeaddrinfo(res);
}

/* ---------------------------------
   HÀM XỬ LÝ MỘT DÒNG NHẬP
   --------------------------------- */
void process_input_line(char *line) {
    char *token = strtok(line, " ");
    while (token) {
        if (is_valid_ipv4(token) || is_valid_ipv6(token))
            resolve_ip(token);
        else
            resolve_domain(token);
        token = strtok(NULL, " ");
    }
}

/* ---------------------------------
   HÀM CHÍNH
   --------------------------------- */
int main(int argc, char *argv[]) {
    char line[256];

    // Nếu có tham số là file batch
    if (argc == 2) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Cannot open file: %s\n", argv[1]);
            return 1;
        }
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\n")] = '\0';
            if (strlen(line) == 0) continue;
            process_input_line(line);
        }
        fclose(f);
        return 0;
    }

    printf("=== DNS Resolver (IPv4/IPv6 supported) ===\n");
    printf("Enter domain or IP (empty line to quit)\n\n");

    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;

        line[strcspn(line, "\n")] = '\0'; // Xóa ký tự newline
        if (strlen(line) == 0) break; // Dừng khi chuỗi rỗng

        process_input_line(line);
    }

    printf("Goodbye!\n");
    return 0;
}
