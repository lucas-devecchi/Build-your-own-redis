#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cassert>
#include "./IO_utils.h"

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

const size_t k_max_msg = 4096; // k is from Konstant.

static int32_t one_request(int connfd)
{
    // 4 bytes header
    char rbuf[4 + k_max_msg];
    errno = 0; // global var used by OS to indicate errors.

    int32_t err = read_full(connfd, rbuf, 4);
    if (err)
    {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }
    uint32_t len = 0;
    
    // side note: memcpy(destination, origin, howManyBytesCopy)
    memcpy(&len, rbuf, 4); // set msg length in "len" with the value of the first byte on buff read.

    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }

    // request body
    err = read_full(connfd, &rbuf[4], len);
    if (err)
    {
        msg("read() error");
        return err;
    }

    // do something
    printf("client says: %.*s\n", len, rbuf[4]);

    // reply using the same protocol
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply); // parse length as uint_32t
    memcpy(wbuf, &len, 4); // assign the 4 bytes for size.
    memcpy(&wbuf[4], reply, len); // assign the payload ("world")
    return write_all(connfd, wbuf, 4 + len);
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