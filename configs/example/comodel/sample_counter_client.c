/*
 * Co-model high level simulation and low level implementation in gem5.
 *
 * A simple client to test the sample counter device in src/comodel/.
 * Run it on local host and connect it to the XPU I/O server in gem5.
 *
 * Chun Yang    <yangchun@mprc.pku.edu.cn>
 *
 * Copyright (C) 2024 Microprocessor R&D Center (MPRC), Peking University
 * All rights reserved.
 */

#include <assert.h>
#include <stddef.h>         // offsetof
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>         // unlink

// I/O packet to read/write XPU register.
enum
{
    IO_PACKET_MAGIC = 0x5E,
    IO_PACKET_DATA_SIZE = 1024,
};

struct IOPacket
{
    struct
    {
        unsigned int is_read : 1;
        unsigned int magic : 15;
        uint16_t addr;
        uint32_t size;
    } head;
    char data[IO_PACKET_DATA_SIZE];
};

static const char* client_path = "/tmp/client.socket";
static int client_socket = -1;
static struct IOPacket client_packet;

static void
usage(char *program)
{
    printf("A simple client to test the co-modeled sample counter.\n");
    printf("Usage: %s <server socket>\n", program);
}

static uint32_t
read_word(uint16_t addr)
{
    int n;
    uint32_t w;

    client_packet.head.is_read = 1;
    client_packet.head.magic = IO_PACKET_MAGIC;
    client_packet.head.addr = addr;
    client_packet.head.size = sizeof(w);
    n = write(client_socket, &client_packet.head, sizeof(client_packet.head));
    assert(n == sizeof(client_packet.head));
    n = read(client_socket, &w, sizeof(w));
    assert(n == sizeof(w));

    return w;
}

void
write_word(uint16_t addr, uint32_t w)
{
    int n;

    client_packet.head.is_read = 0;
    client_packet.head.magic = IO_PACKET_MAGIC;
    client_packet.head.addr = addr;
    client_packet.head.size = sizeof(w);
    n = write(client_socket, &client_packet.head, sizeof(client_packet.head));
    assert(n == sizeof(client_packet.head));
    n = write(client_socket, &w, sizeof(w));
    assert(n == sizeof(w));
}

void
close_command()
{
    int n;
    client_packet.head.is_read = 0;
    client_packet.head.magic = IO_PACKET_MAGIC;
    client_packet.head.addr = 0;
    client_packet.head.size = 0;
    n = write(client_socket, &client_packet.head, sizeof(client_packet.head));
    assert(n == sizeof(client_packet.head));
}

int
main(int argc, char* argv[])
{
    char server_path[256];
    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    strncpy(server_path, argv[1], sizeof(server_path));

    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("[Error] Failed to open socket");
        exit(EXIT_FAILURE);
    }

    // Bind the UNIX domain socket with an explict path.
    struct sockaddr_un sa_client;
    int sa_size;
    memset(&sa_client, 0, sizeof(sa_client));
    sa_client.sun_family = AF_UNIX;
    strcpy(sa_client.sun_path, client_path);
    sa_size = offsetof(struct sockaddr_un, sun_path) + strlen(client_path);
    unlink(client_path);
    if (bind(client_socket, (struct sockaddr *)&sa_client, sa_size) < 0) {
        perror("[Error] Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un sa_server;
    memset(&sa_server, 0, sizeof(sa_server));
    sa_server.sun_family = AF_UNIX;
    strcpy(sa_server.sun_path, server_path);
    sa_size = offsetof(struct sockaddr_un, sun_path) + strlen(server_path);
    if (connect(client_socket, (struct sockaddr *)&sa_server, sa_size) < 0) {
        perror("[Error] Failed to connect to server");
        exit(EXIT_FAILURE);
    }
    printf("Connected to XPU I/O server at '%s'.\n", server_path);

    uint32_t w;
    write_word(0x0, 0);   // reset
    w = read_word(0x0);
    printf("Reset the counter: counter=%d\n", w);
    write_word(0x4, 5);   // count
    w = read_word(0x0);
    printf("Count +5: counter=%d\n", w);
    write_word(0x4, 6);   // count
    w = read_word(0x0);
    printf("Count +6: counter=%d\n", w);
    write_word(0x4, 7);   // count
    w = read_word(0x0);
    printf("Count +7: counter=%d\n", w);
    close_command();

    close(client_socket);
    client_socket = -1;

    return 0;
}
