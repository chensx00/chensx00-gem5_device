/*
 * Co-model high level simulation and low level implementation in gem5.
 *
 * I/O server for a simulated accelerator (XPU) in gem5.
 * 1. XPU connects to the system bus of gem5.
 * 2. SE workload calls an emulated driver to map the XPU address range.
 * 3. This server runs on gem5 CPU and provides memory mapped I/O over
 *    a UNIX socket.
 *
 * Chun Yang    <yangchun@mprc.pku.edu.cn>
 *
 * Copyright (C) 2024 Microprocessor R&D Center (MPRC), Peking University
 * All rights reserved.
 */

#include <assert.h>
#include <fcntl.h>
#include <stddef.h>         // offsetof
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>         // unlink

#define LOG     "[--XPU Server--] "

#define XPU_MMIO_DEVICE     "/dev/se_mmio"

// Mapped address for XPU I/O.
void *xpu_mmio = NULL;
int xpu_mmio_size = -1;

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

static void
usage(char *program)
{
    printf("I/O server for a simulated accelerator (XPU).\n");
    printf("Usage: %s <Accelerator name> <I/O range size>\n", program);
}

static void
map_xpu_io(int io_size)
{
    int fd = -1;
    fd = open(XPU_MMIO_DEVICE, O_RDWR);
    if (fd < 0) {
        perror(LOG "[Error] Failed to open MMIO device for XPU");
        exit(EXIT_FAILURE);
    }

    assert(xpu_mmio == NULL && xpu_mmio_size == -1);
    xpu_mmio = mmap(0, io_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (xpu_mmio == MAP_FAILED) {
        perror(LOG "[Error] Failed to map device I/O for XPU");
        exit(EXIT_FAILURE);
    }
    xpu_mmio_size = io_size;
    printf(LOG "Mapped device I/O to %p.\n", xpu_mmio);

    close(fd);
    fd = -1;
}

static void
unmap_xpu_io()
{
    assert(xpu_mmio != NULL && xpu_mmio_size > 0);
    munmap(xpu_mmio, xpu_mmio_size);
    xpu_mmio = NULL;
    xpu_mmio_size = -1;
}

int main(int argc, char *argv[])
{
    char server_path[256];
    if (argc != 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    snprintf(server_path, sizeof(server_path),
             "%s-server.socket", argv[1]);
    int io_size = atoi(argv[2]);
    if (io_size <= 0) {
        fprintf(stderr, LOG "Bad I/O range size '%s'\n", argv[2]);
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    printf(LOG "I/O server for simulated accelerator %s\n", argv[1]);

    map_xpu_io(io_size);

    int listen_fd = -1;
    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror(LOG "[Error] Failed to open socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un sa_server;
    int sa_size;
    memset(&sa_server, 0, sizeof(sa_server));
    sa_server.sun_family = AF_UNIX;
    strcpy(sa_server.sun_path, server_path);
    sa_size = offsetof(struct sockaddr_un, sun_path) + strlen(server_path);
    unlink(server_path);
    if (bind(listen_fd, (struct sockaddr *)&sa_server, sa_size) < 0) {
        perror(LOG "[Error] Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 1) < 0) {
        perror(LOG "[Error] Failed to listen socket");
        exit(EXIT_FAILURE);
    }
    printf(LOG "Listening at '%s'...\n", server_path);
    fflush(stdout);

    struct sockaddr_un sa_client;
    int conn_fd = -1;
    sa_size = sizeof(sa_client);
    conn_fd = accept(listen_fd, (struct sockaddr *)&sa_client,
                     (socklen_t *)&sa_size);
    if (conn_fd < 0) {
        perror(LOG "[Error] Failed to accept socket");
        exit(EXIT_FAILURE);
    }
    printf(LOG "Accepted client '%s'.\n", sa_client.sun_path);
    fflush(stdout);

    struct IOPacket packet;
    while (1) {
        int n;
        n = read(conn_fd, &packet.head, sizeof(packet.head));
        if (n < 0) {
            perror(LOG "[Error] Failed to receive I/O packet head");
            break;
        } else if (n == 0) {
            printf(LOG "Client '%s' closed.\n", sa_client.sun_path);
            break;
        }
        assert(n == sizeof(packet.head));

        if (packet.head.magic != IO_PACKET_MAGIC) {
            printf(LOG "[Error] Bad magic 0x%x in I/O packet.\n",
                    packet.head.magic);
            break;
        }

        // 0-size request indicates closing the connection.
        if (packet.head.size == 0) {
            printf(LOG "Client '%s' closed.\n", sa_client.sun_path);
            break;
        }

        if (packet.head.is_read) {
            assert(packet.head.size <= sizeof(packet.data));
            assert(packet.head.addr % 4 == 0 && packet.head.size % 4 == 0);
            // TODO pad in data to align the size
            for (int i = 0; i < packet.head.size; i += 4) {
                *(uint32_t *)(packet.data + i)
                    = *(uint32_t *)(xpu_mmio + packet.head.addr + i);
            }
            n = write(conn_fd, packet.data, packet.head.size);
            if (n < packet.head.size) {
                printf(LOG "[Error] Failed to send I/O packet data");
                break;
            }
            printf(LOG "  Read:  [0x%04x | %4d]\t %4d bytes\n",
                    packet.head.addr, packet.head.addr, packet.head.size);
        } else {
            assert(packet.head.size <= sizeof(packet.data));
            assert(packet.head.addr % 4 == 0 && packet.head.size % 4 == 0);
            n = read(conn_fd, packet.data, packet.head.size);
            if (n < packet.head.size) {
                printf(LOG "[Error] Failed to receive I/O packet data");
                break;
            }
            // TODO pad in data to align the size
            for (int i = 0; i < packet.head.size; i += 4) {
                *(uint32_t *)(xpu_mmio + packet.head.addr + i)
                    = *(uint32_t *)(packet.data + i);
            }
            printf(LOG "  Write: [0x%04x | %4d]\t %4d bytes\n",
                    packet.head.addr, packet.head.addr, packet.head.size);
        }
    }
    close(conn_fd);
    conn_fd = -1;
    printf(LOG "------------------------------------------\n\n");
    fflush(stdout);

    close(listen_fd);
    listen_fd = -1;

    unmap_xpu_io();

    return 0;
}
