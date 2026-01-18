#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "./IO_utils.h"
#include "../utils.h"

using namespace std;

static int32_t query(int fd, const char *text)
{
    uint32_t len = (int32_t)strlen(text);
    if (len > k_max_msg)
    {
        return -1;
    }

    // send request
    char wbuf[4 + k_max_msg];
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);
    if (int32_t err = write_all(fd, wbuf, 4 + len))
    {
        return err;
    }

    // 4 bytes header
    char rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err)
    {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }
    memcpy(&len, rbuf, 4);
    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }

    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err)
    {
        msg("read() error");
        return err;
    }

    // do something
    printf("server says: %.*s\n", len, &rbuf[4]);
    return 0;
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP Socket
    if (fd < 0)
    {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1024);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);                      // 127.0.0.1
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr)); // Assign address to Socket
    if (rv)
    {
        die("connect");
    }

    // send multiple requests
    int32_t err = query(fd, "hello1");

    if (!err)
        err = query(fd, "hello2");

    close(fd);
    return err;
}