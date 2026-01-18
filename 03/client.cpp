#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

using namespace std;

static void msg(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
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

    char msg[] = "hello";
    write(fd, msg, strlen(msg)); // Write "hello" through socket

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1); // Read from sv.
    if (n < 0)
    {
        die("read");
    }
    printf("server says: %s\n", rbuf);
    close(fd);
    return 0;
}