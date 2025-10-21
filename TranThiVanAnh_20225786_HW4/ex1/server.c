#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <Port_Number>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    //socket lắng nghe và chấp nhận kết nối từ client
    int listenfd, connfd;
    struct sockaddr_in servaddr, clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    char buff[BUFF_SIZE + 1];
    char letters[BUFF_SIZE], digits[BUFF_SIZE];
    int bytes_received;
    // tạo socket để lắng nghe
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket() failed");
        return 1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // ipv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // lắng nghe trên tất cả các giao diện mạng
    servaddr.sin_port = htons(port);

    
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind() failed");
        return 1;
    }
    //tối đa số 5 kết nối chờ
    if (listen(listenfd, 5) < 0) {
        perror("listen() failed");
        return 1;
    }

    printf("📡 Server listening on port %d...\n", port);

    while (1) {
        //chấp nhận kết nối từ client
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (connfd < 0) {
            perror("accept() failed");
            continue;
        }

        printf("👉 Client connected: %s:%d\n",
               inet_ntoa(clientaddr.sin_addr),
               ntohs(clientaddr.sin_port));

        while ((bytes_received = recv(connfd, buff, BUFF_SIZE, 0)) > 0) {
            buff[bytes_received] = '\0';

            // Nếu client gửi chuỗi rỗng => ngắt kết nối
            if (strcmp(buff, "") == 0)
                break;

            int valid = 1;
            int l_idx = 0, d_idx = 0;

            for (int i = 0; i < bytes_received; i++) {
                if (isalpha(buff[i])) letters[l_idx++] = buff[i];
                else if (isdigit(buff[i])) digits[d_idx++] = buff[i];
                else { valid = 0; break; }
            }

            letters[l_idx] = '\0';
            digits[d_idx] = '\0';

            char response[BUFF_SIZE * 2];

            if (!valid) {
                snprintf(response, sizeof(response),
                         "Error: Invalid character detected");
            } else {
                snprintf(response, sizeof(response),
                         "Letters: %s | Digits: %s", letters, digits);
            }

            send(connfd, response, strlen(response), 0);
        }

        printf("❌ Client disconnected.\n");
        close(connfd);
    }

    close(listenfd);
    return 0;
}
