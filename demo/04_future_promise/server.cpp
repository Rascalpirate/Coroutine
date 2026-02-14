#include <arpa/inet.h>
#include <future>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

std::future<ssize_t> async_recv(int fd, char *buf, size_t len)
{
    std::promise<ssize_t> p;
    auto                  f = p.get_future();
    std::thread(
        [fd, buf, len, p = std::move(p)]() mutable
        {
            p.set_value(::recv(fd, buf, len, 0));
        })
        .detach();
    return f;
}

int main()
{
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    int opt       = 1;
    ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(9004);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ::bind(listen_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    ::listen(listen_fd, 64);

    std::cout << "[future/promise server] listen on 9004\n";
    while (true)
    {
        int conn = ::accept(listen_fd, nullptr, nullptr);
        std::thread(
            [conn]()
            {
                char buf[1024];
                while (true)
                {
                    auto    fut = async_recv(conn, buf, sizeof(buf));
                    ssize_t n   = fut.get();
                    if (n <= 0)
                        break;
                    ::send(conn, buf, static_cast<size_t>(n), 0);
                }
                ::close(conn);
            })
            .detach();
    }
}
