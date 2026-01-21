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
#include <map>

using std::map;
using std::vector;
using namespace std;

struct Response
{
    uint32_t status = 0;
    vector<uint8_t> data;
};

// Response::status
enum
{
    RES_OK = 0,
    RES_ERR = 1, // error
    RES_NX = 2,  // key not found
};

// placeholder; implemented later
static map<string, string> g_data;

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
    cur += n;                 // set pointer ahead of substring.
    return true;
}

// Step 1: Parse the request command
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

// Step 2: Process the command
static void do_request(vector<string> &cmd, Response &out)
{
    string action = cmd[0];
    string key = cmd[1];
    string value = cmd[2];

    // GET
    if (cmd.size() == 2 && action == "get")
    {
        auto it = g_data.find(key);
        if (it == g_data.end())
        {
            out.status = RES_NX; // not found
            return;
        }
        const std::string &val = it->second;
        out.data.assign(val.begin(), val.end());
        return;
    }

    // SET
    if (cmd.size() == 3 && action == "set")
    {
        g_data[key].swap(value); // changes the old pointer for value pointer
        return;
    }

    // DELETE
    if (cmd.size() == 2 && action == "del")
    {
        g_data.erase(key);
        return;
    }

    out.status = RES_ERR; // unrecognized command
}

// Step 3: Serialize the response

static void make_response(const Response &resp, vector<uint8_t> &out)
{
    uint32_t resp_len = 4 + (uint32_t)resp.data.size();
    buf_append(out, (const uint8_t *)&resp_len, 4);
    buf_append(out, (const uint8_t *)&resp.status, 4);
    buf_append(out, resp.data.data(), resp.data.size());
}

int main()
{
    // the listening socket
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

    fd_set_nb(listeningFd);

    // listen
    rv = listen(listeningFd, SOMAXCONN);
    if (rv)
        die("listen()");

    // a map of all client connections, indexed by fd
    vector<Conn *> fd2conn;

    // The event loop
    vector<struct pollfd> poll_args;
    while (true)
    {
        // prepare the arguments of the poll()
        poll_args.clear();

        // put the listening sockets in the first position
        struct pollfd pfd = {listeningFd, POLLIN, 0};
        poll_args.push_back(pfd);

        // the rest are connection sockets
        for (Conn *conn : fd2conn)
        {
            if (!conn)
                continue;

            struct pollfd pfd = {conn->fd, POLLERR, 0};

            // poll() flags from the application's intent

            /*
                |= is a Logic OR, 0100 | 0001 equals 0101
                Can keep the POLLIN when assign POLLOUT.
                With only =, POLLIN would be removed (if it was assigned before).
            */
            if (conn->want_read)
                pfd.events |= POLLIN;

            if (conn->want_write)
                pfd.events |= POLLOUT;

            poll_args.push_back(pfd);
        }

        // wait for readinesss
        int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), -1);
        if (rv < 0)
        {
            if (errno == EINTR) // when syscall is aborted by Unix, it returns EINTR. must iterate.
                continue;
            die("poll");
        }

        // handle the listening socket
        if (poll_args[0].revents)
        {
            if (Conn *conn = handle_accept(listeningFd))
            {
                // put it into the map (resize if needed)
                if (fd2conn.size() <= (size_t)conn->fd)
                    fd2conn.resize(conn->fd + 1);
                fd2conn[conn->fd] = conn;
            }
        }

        for (size_t i = 1; i < poll_args.size(); i++)
        {
            uint32_t ready = poll_args[i].revents;
            Conn *conn = fd2conn[poll_args[i].fd];
            if (ready & POLLIN)
                handle_read(conn);
            if (ready & POLLOUT)
                handle_write(conn);

            if ((ready & POLLERR) || conn->want_close)
            {
                (void)close(conn->fd);
                fd2conn[conn->fd] = NULL;
                delete conn;
            }
        }
    }
    return 0;
}