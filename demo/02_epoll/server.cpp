#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_set>

namespace
{
int set_nonblocking(int fd)
{
    int flags = ::fcntl(fd, F_GETFL, 0);
    return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
} // namespace

int main()
{
    constexpr int kPort     = 9002;
    int           listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    int           opt       = 1;
    ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(listen_fd);

    sockaddr_in addr {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(kPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ::bind(listen_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    ::listen(listen_fd, 128);

    int         epfd = ::epoll_create1(0);
    epoll_event ev {};
    ev.events  = EPOLLIN;
    ev.data.fd = listen_fd;
    ::epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    std::unordered_set<int> clients;
    std::cout << "[epoll server] listen on " << kPort << '\n';

    epoll_event events[64];
    while (true)
    {
        int n = ::epoll_wait(epfd, events, 64, -1);
        for (int i = 0; i < n; ++i)
        {
            int fd = events[i].data.fd;
            if (fd == listen_fd)
            {
                while (true)
                {
                    int conn = ::accept(listen_fd, nullptr, nullptr);
                    if (conn < 0)
                        break;
                    set_nonblocking(conn);
                    epoll_event cev {};
                    cev.events  = EPOLLIN | EPOLLRDHUP;
                    cev.data.fd = conn;
                    ::epoll_ctl(epfd, EPOLL_CTL_ADD, conn, &cev);
                    clients.insert(conn);
                }
            }
            else
            {
                char    buf[1024];
                ssize_t rn = ::recv(fd, buf, sizeof(buf), 0);
                if (rn > 0)
                {
                    ::send(fd, buf, static_cast<size_t>(rn), 0);
                }
                else
                {
                    ::epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                    ::close(fd);
                    clients.erase(fd);
                }
            }
        }
    }
}
