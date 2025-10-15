#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>

#define BUFF_SIZE 1024

// Kiểm tra chuỗi có phải là IP hợp lệ không
int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        return 1;
    }

    int port_num = atoi(argv[1]);
    if (port_num <= 0 || port_num > 65535) {
        printf("Invalid port number\n");
        return 1;
    }

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buff[BUFF_SIZE];
    socklen_t len = sizeof(cliaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port_num);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    printf("UDP Resolver Server running on port %d...\n", port_num);

    while (1) {
        int n = recvfrom(sockfd, buff, BUFF_SIZE - 1, 0,
                         (struct sockaddr*)&cliaddr, &len);
        if (n <= 0) continue;
        buff[n] = '\0';

        printf("Received from %s:%d → %s\n",
               inet_ntoa(cliaddr.sin_addr),
               ntohs(cliaddr.sin_port),
               buff);

        char response[BUFF_SIZE];
        memset(response, 0, sizeof(response));

        // Xử lý nếu là IP
        if (is_valid_ip(buff)) {
            struct in_addr addr;
            inet_pton(AF_INET, buff, &addr);
            struct hostent *host = gethostbyaddr(&addr, sizeof(addr), AF_INET);

            if (host) {
                snprintf(response, sizeof(response),
                         "Official name: %s\nAlias name:\n", host->h_name);
                for (char **a = host->h_aliases; *a; a++) {
                    strcat(response, *a);
                    strcat(response, "\n");
                }
            } else {
                snprintf(response, sizeof(response), "Not found information");
            }
        }
        // Nếu không phải IP → coi là domain
        else {
            struct hostent *host = gethostbyname(buff);
            if (!host) {
                snprintf(response, sizeof(response), "Not found information");
            } else {
                snprintf(response, sizeof(response), "Official IP: %s\nAlias IP:\n",
                         inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
                for (int i = 1; host->h_addr_list[i] != NULL; i++) {
                    strcat(response, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
                    strcat(response, "\n");
                }
            }
        }

        // Kiểm tra IP sai định dạng (ví dụ 259.12.34.12)
        int a, b, c, d;
        if (sscanf(buff, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
            if (a > 255 || b > 255 || c > 255 || d > 255) {
                snprintf(response, sizeof(response), "IP Address is invalid");
            }
        }

        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr*)&cliaddr, len);
    }

    close(sockfd);
    return 0;
}
