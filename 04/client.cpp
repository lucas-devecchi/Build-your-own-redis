#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "./IO_utils.h"
#include "../utils.h"

using namespace std;


const size_t k_max_msg = 4096; // k is from Konstant.

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
    if(int32_t err = write_all(fd,wbuf,4+len)){
        return err;
    }
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