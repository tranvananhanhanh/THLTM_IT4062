#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <Server_IP> <Port_Number>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int clientfd;
    struct sockaddr_in servaddr;
    char buff[BUFF_SIZE], recv_buff[BUFF_SIZE + 1];
    int ret;

    // Step 1: Create socket
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0) {
        perror("socket() failed");
        return 1;
    }

    // Step 2: Set server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_ip);
    servaddr.sin_port = htons(port);

    // Step 3: Connect to server
    if (connect(clientfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect() failed");
        close(clientfd);
        return 1;
    }

    printf("✅ Connected to server %s:%d\n", server_ip, port);

    // Step 4: Communicate repeatedly
    while (1) {
        printf("Enter string (empty to quit): ");
        fflush(stdout);
        fgets(buff, sizeof(buff), stdin);
        buff[strcspn(buff, "\n")] = 0;

        if (strlen(buff) == 0) break;

        send(clientfd, buff, strlen(buff), 0);
        ret = recv(clientfd, recv_buff, BUFF_SIZE, 0);

        if (ret > 0) {
            recv_buff[ret] = '\0';
            printf("Server reply: %s\n", recv_buff);
        }
    }

    // Step 5: Close connection
    close(clientfd);
    printf("❌ Connection closed.\n");
    return 0;
}
