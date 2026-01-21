#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include "../utils.h"
#include <assert.h>

using std::vector;
using namespace std;

static bool read_u32(const uint8_t *&cur, const uint8_t *end, uint32_t &out)
{
    if (cur + 4 > end)
    {
        return false;
    }
    memcpy(&out, cur, 4);
    cur += 4;
    return true;
}

static bool read_str(const uint8_t *&cur, const uint8_t *end, size_t n, string &out)
{
    if (cur + n > end)
    {
        return false;
    }
    out.assign(cur, cur + n); // substring
    cur += n; // set pointer ahead of substring.
    return true;
}

static int32_t parse_req(const uint8_t *data, size_t size, vector<string> &out)
{
    const uint8_t *end = data + size;
    uint32_t nstr = 0;

    // saves in nstr the value the first 4 bytes
    if (!read_u32(data, end, nstr))
        return -1;

    if (nstr > k_max_args)
        return -1; // safety limit

    while (out.size() < nstr)
    {
        uint32_t len = 0;
        if (!read_u32(data, end, len)) // nstr len string len string. Saving len amount.
            return -1;

        out.push_back(string()); // out = [..., ""]

        if (!read_str(data, end, len, out.back()))
            return -1;
    }

    if (data != end)
        return -1; // trailing garbage

    return 0;
}

int main()
{
    int listeningFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningFd < 0)
        die("socket()");

    int val = 1;
    setsockopt(listeningFd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0
    int rv = bind(listeningFd, (const sockaddr *)&addr, sizeof(addr));
    if (rv)
        die("bind()");

    return 0;
}