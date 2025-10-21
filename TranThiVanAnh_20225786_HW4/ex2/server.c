#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define BUFF_SIZE 4096
#define SAVE_DIR "./upload/"   // thư mục lưu file

void ensure_upload_dir() {
    struct stat st = {0};
    if (stat(SAVE_DIR, &st) == -1) {
        mkdir(SAVE_DIR, 0700);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <Port_Number>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int listenfd, connfd;
    struct sockaddr_in servaddr, clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    char filename[256], filepath[512];
    char buff[BUFF_SIZE];
    int bytes_received;

    ensure_upload_dir();

    // ===== Step 1: Tạo socket =====
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Error: Cannot create socket");
        return 1;
    }

    // ===== Step 2: Bind và Listen =====
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Error: Cannot bind");
        close(listenfd);
        return 1;
    }

    if (listen(listenfd, 5) < 0) {
        perror("Error: Cannot listen");
        close(listenfd);
        return 1;
    }

    printf("📡 Server listening on port %d...\n", port);

    // ===== Step 3: Chờ client =====
    while (1) {
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (connfd < 0) {
            perror("Error: Cannot accept");
            continue;
        }

        printf("👉 Connected from %s:%d\n",
               inet_ntoa(clientaddr.sin_addr),
               ntohs(clientaddr.sin_port));

        // Nhận tên file
        memset(filename, 0, sizeof(filename));
        bytes_received = recv(connfd, filename, sizeof(filename) - 1, 0);
        if (bytes_received <= 0) {
            close(connfd);
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s%s", SAVE_DIR, filename);

        // Kiểm tra trùng tên file
        if (access(filepath, F_OK) == 0) {
            char *msg = "Error: File is existent on server";
            send(connfd, msg, strlen(msg), 0);
            printf("❌ %s\n", msg);
            close(connfd);
            continue;
        }

        // Gửi tín hiệu OK
        char *okmsg = "OK";
        send(connfd, okmsg, strlen(okmsg), 0);

        // Mở file để ghi
        FILE *f = fopen(filepath, "wb");
        if (f == NULL) {
            perror("Error: Cannot create file");
            close(connfd);
            continue;
        }

        printf("⬇ Receiving file: %s ...\n", filename);
        int total = 0;

        // Nhận dữ liệu file
        while ((bytes_received = recv(connfd, buff, BUFF_SIZE, 0)) > 0) {
            // ghi từng byte, không xử lý văn bản
            fwrite(buff, 1, bytes_received, f);
            total += bytes_received;
        }

        fclose(f);
        printf("✅ Received %d bytes, saved to %s\n", total, filepath);

        close(connfd);
    }

    close(listenfd);
    return 0;
}
