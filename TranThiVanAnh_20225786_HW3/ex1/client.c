#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFF_SIZE 65535

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s IPAddress PortNumber\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char buff[BUFF_SIZE];
    socklen_t len = sizeof(servaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    while (1) {
        printf("Enter message: ");
        fgets(buff, BUFF_SIZE, stdin);

        // Xóa ký tự xuống dòng
        buff[strcspn(buff, "\n")] = 0;

        if (strlen(buff) == 0) break;

        sendto(sockfd, buff, strlen(buff), 0,
               (struct sockaddr*)&servaddr, len);

        int n = recvfrom(sockfd, buff, BUFF_SIZE, 0, NULL, NULL);
        buff[n] = '\0';
        printf("%s\n", buff);
    }

    close(sockfd);
    return 0;
}
