#include <arpa/inet.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

void send_cps(int fd, std::string msg, std::function<void()> k)
{
    std::thread(
        [fd, msg = std::move(msg), k = std::move(k)]()
        {
            ::send(fd, msg.data(), msg.size(), 0);
            k();
        })
        .detach();
}

int main()
{
    int         fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(9005);
    ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    send_cps(
        fd,
        "hello from cps client\n",
        [fd]()
        {
            char    buf[1024] {};
            ssize_t n = ::recv(fd, buf, sizeof(buf) - 1, 0);
            if (n > 0)
                std::cout << "echo: " << buf;
            ::close(fd);
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
