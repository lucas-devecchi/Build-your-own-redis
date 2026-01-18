#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

using namespace std;

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void do_something(int connfd) {
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1); // Reads socket paylaod
    if (n < 0) {
        msg("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
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
    if(rv) die("bind()");

    // listen
    rv = listen(fd, SOMAXCONN); // SOMAXCONN = 4096, size of the queue.
    if (rv) { die("listen()"); }

    while (true)
    {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen); // It isn't const struct bc write can change it.
        if (connfd < 0)
            continue;

        do_something(connfd);
        close(connfd);
    }
    return 0;
}