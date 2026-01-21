#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

const size_t k_max_msg = 4096;
const size_t k_max_args = 200 * 1000;

// Log a msg
static void msg(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

// Log an error and abort
static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void msg_errno(const char *msg)
{
    fprintf(stderr, "[errno:%d] %s\n", errno, msg);
}

int tcp_connect(uint32_t ip_addr, uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
        return -1;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    // Como INADDR_LOOPBACK ya está en Host Byte Order,
    // usamos htonl para asegurar Network Byte Order
    addr.sin_addr.s_addr = htonl(ip_addr);

    if (connect(fd, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        die("connect");
        return -1;
    }
    return fd;
}


#endif