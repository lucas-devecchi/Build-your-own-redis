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