#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#define BUFF_SIZE 65535

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s PortNumber\n", argv[0]);
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

    printf("UDP Echo Server running on port %d...\n", port_num);

    while (1) {
        int n = recvfrom(sockfd, buff, BUFF_SIZE, 0,
                         (struct sockaddr*)&cliaddr, &len);
        buff[n] = '\0';
        printf("Received from %s:%d → %s\n",
               inet_ntoa(cliaddr.sin_addr),
               ntohs(cliaddr.sin_port),
               buff);

        char letters[BUFF_SIZE] = {0};
        char digits[BUFF_SIZE] = {0};
        int l = 0, d = 0;
        int valid = 1;

        for (int i = 0; i < n; i++) {
            if (isalpha((unsigned char)buff[i])) {
                letters[l++] = buff[i];
            } else if (isdigit((unsigned char)buff[i])) {
                digits[d++] = buff[i];
            } else {
                valid = 0;
                break;
            }
        }

        char response[BUFF_SIZE];
        if (!valid) {
            snprintf(response, sizeof(response), "Error: Input contains invalid characters.");
        } else {
            snprintf(response, sizeof(response), "%s\n%s", letters, digits);
        }

        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr*)&cliaddr, len);
    }
}
