#include <arpa/inet.h>
#include <future>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

std::future<void> async_send(int fd, const std::string msg)
{
    std::promise<void> p;
    auto               f = p.get_future();
    std::thread(
        [fd, msg, p = std::move(p)]() mutable
        {
            ::send(fd, msg.data(), msg.size(), 0);
            p.set_value();
        })
        .detach();
    return f;
}

int main()
{
    int         fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(9004);
    ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    auto sent = async_send(fd, "hello from future/promise client\n");
    sent.wait();

    char    buf[1024] {};
    ssize_t n = ::recv(fd, buf, sizeof(buf) - 1, 0);
    if (n > 0)
        std::cout << "echo: " << buf;
    ::close(fd);
}
