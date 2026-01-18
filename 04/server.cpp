#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cassert>

using namespace std;

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

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


int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;
    // Cancel the status WAIT_TIME on socket so it can be reused.
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;       // Uses IPv4
    addr.sin_port = htons(1024);     // Uses port n°1024. htons() parses little to big-endian.
    addr.sin_addr.s_addr = htonl(0); // Uses all the IP's from host. Works like htons but for 32bits.

    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr)); // Give address to socket
    if (rv)
        die("bind()");

    // listen
    rv = listen(fd, SOMAXCONN); // SOMAXCONN = 4096, size of the queue.
    if (rv)
    {
        die("listen()");
    }

    while (true)
    {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen); // It isn't const struct bc write can change it.
        if (connfd < 0)
            continue;

        // only serves one client connection at once
        while (true)
        {
            int32_t err = 3; // Provisorio
            //  one_request(connfd);
            if (err)
            {
                break;
            }
        }

        close(connfd);
    }
    return 0;
}