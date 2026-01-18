#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <unistd.h>  // read, write
#include <assert.h>  // assert
#include <stdint.h>  // int32_t
#include <stddef.h>  // size_t

/*
    fd: socket to read
    buf: buffer where we save bytes read
    n: amount of bytes to read.
*/
static int32_t read_full(int fd, char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0)
            return -1; // error or unexpected EOF

        assert((size_t)rv <= n); // if the condition fails, the program aborts.
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

/*
    fd: socket to read
    buf: buffer where we save bytes read
    n: amount of bytes to read.
*/
static int32_t write_all(int fd, const char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0)
            return -1;
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

#endif // IO_UTILS_H