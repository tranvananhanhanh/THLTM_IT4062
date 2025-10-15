CC = gcc
CFLAGS = -Wall -Wextra -O2

# Các chương trình cần build
TARGETS = client server

all: $(TARGETS)

udp_client: udp_client.c
	$(CC) $(CFLAGS) -o client client.c

udp_server: udp_server.c
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm -f $(TARGETS)
