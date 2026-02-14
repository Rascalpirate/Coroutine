#include <arpa/inet.h>
#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using Continuation = std::function<void(ssize_t)>;

void recv_cps(int fd, char *buf, size_t len, Continuation k)
{
    std::thread(
        [=]() mutable
        {
            ssize_t n = ::recv(fd, buf, len, 0);
            k(n);
        })
        .detach();
}

void loop_echo_cps(int conn)
{
    auto                  buf = std::make_shared<std::array<char, 1024>>();
    std::function<void()> step;
    step = [conn, buf, &step]() mutable
    {
        recv_cps(
            conn,
            buf->data(),
            buf->size(),
            [conn, buf, &step](ssize_t n) mutable
            {
                if (n <= 0)
                {
                    ::close(conn);
                    return;
                }
                ::send(conn, buf->data(), static_cast<size_t>(n), 0);
                step();
            });
    };
    step();
}

int main()
{
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    int opt       = 1;
    ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(9005);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ::bind(listen_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    ::listen(listen_fd, 64);

    std::cout << "[cps server] listen on 9005\n";
    while (true)
    {
        int conn = ::accept(listen_fd, nullptr, nullptr);
        if (conn >= 0)
            loop_echo_cps(conn);
    }
}
