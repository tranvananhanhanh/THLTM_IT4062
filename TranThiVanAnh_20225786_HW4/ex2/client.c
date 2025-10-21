#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define BUFF_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <Server_IP> <Port_Number>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    char filepath[512], filename[256];
    struct sockaddr_in servaddr;
    int clientfd;
    char buff[BUFF_SIZE];
    int bytes_sent, bytes_read;

    while (1) {
        printf("Enter file path (empty to quit): ");
        fflush(stdout);
        fgets(filepath, sizeof(filepath), stdin);
        filepath[strcspn(filepath, "\n")] = 0;  // bỏ \n

        if (strlen(filepath) == 0)
            break;

        FILE *f = fopen(filepath, "rb");
        if (f == NULL) {
            printf("❌ Error: File not found\n");
            continue;
        }

        // Lấy tên file từ đường dẫn
        char *basename = strrchr(filepath, '/');
        if (basename == NULL)
            basename = filepath;
        else
            basename++;

        strncpy(filename, basename, sizeof(filename));

        // ===== Kết nối đến server =====
        clientfd = socket(AF_INET, SOCK_STREAM, 0);
        if (clientfd < 0) {
            perror("Error: Cannot create socket");
            fclose(f);
            continue;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(server_ip);
        servaddr.sin_port = htons(port);

        if (connect(clientfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("Error: Cannot connect");
            fclose(f);
            close(clientfd);
            continue;
        }

        // Gửi tên file
        send(clientfd, filename, strlen(filename), 0);

        // Nhận phản hồi từ server (OK hoặc Error)
        char response[128] = {0};
        recv(clientfd, response, sizeof(response) - 1, 0);

        if (strncmp(response, "OK", 2) != 0) {
            printf("❌ %s\n", response);
            fclose(f);
            close(clientfd);
            continue;
        }

        // Gửi nội dung file
        printf("⬆ Sending file: %s\n", filename);
        int total = 0;
        while ((bytes_read = fread(buff, 1, BUFF_SIZE, f)) > 0) {
            bytes_sent = send(clientfd, buff, bytes_read, 0);
            total += bytes_sent;
        }

        fclose(f);
        printf("✅ Successful transferring (%d bytes)\n", total);
        close(clientfd);
    }

    return 0;
}
