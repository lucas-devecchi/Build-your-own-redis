#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include "../utils.h"
#include "./IO_utils.h"
#include <assert.h>
using std::vector;
using namespace std;

// append to the back
static void buf_append(std::vector<uint8_t> &buf, const uint8_t *data, size_t len)
{
    buf.insert(buf.end(), data, data + len);
}

// the `query` function was simply splited into `send_req` and `read_res`.
static int32_t send_req(int fd, const uint8_t *text, size_t len)
{
    if (len > k_max_msg)
    {
        return -1;
    }

    vector<uint8_t> wbuf;
    buf_append(wbuf, (const uint8_t *)&len, 4);
    buf_append(wbuf, text, len);
    return write_all(fd, (const char *)wbuf.data(), wbuf.size());
}
static int32_t read_res(int fd)
{
    // 4 bytes header
    std::vector<uint8_t> rbuf;
    rbuf.resize(4);
    errno = 0;
    int32_t err = read_full(fd, (char *)&rbuf[0], 4);
    if (err)
    {
        if (errno == 0)
        {
            msg("EOF");
        }
        else
        {
            msg("read() error");
        }
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf.data(), 4); // assume little endian
    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }

    // reply body
    rbuf.resize(4 + len);
    err = read_full(fd, (char *)&rbuf[4], len);
    if (err)
    {
        msg("read() error");
        return err;
    }

    // do something
    printf("len:%u data:%.*s\n", len, len < 100 ? len : 100, &rbuf[4]);
    return 0;
}

int main()
{

    int fd = tcp_connect(INADDR_LOOPBACK, 1234);
    if (fd < 0)
    {
        cout << "something went wrong on tcp connection" << endl;
        return -1;
    }

    // multiple pipelined requests
    vector<string> query_list = {
        "hello1",
        "hello2",
        "hello3",
        // a large message requires multiple event loop iterations
        string(k_max_msg, 'z'),
        "hello5",
    };
    for (const string &s : query_list)
    {
        int32_t err = send_req(fd, (uint8_t *)s.data(), s.size());
        if (err)
        {
            goto L_DONE;
        }
    }
    for (size_t i = 0; i < query_list.size(); ++i)
    {
        int32_t err = read_res(fd);
        if (err)
        {
            goto L_DONE;
        }
    }

L_DONE:
    close(fd);
    return 0;
}