#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    constexpr int kPort = 9002;
    int           fd    = ::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(kPort);
    ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    const char *msg = "hello from epoll client\n";
    ::send(fd, msg, 24, 0);

    char    buf[1024] {};
    ssize_t n = ::recv(fd, buf, sizeof(buf) - 1, 0);
    if (n > 0)
        std::cout << "echo: " << buf;

    ::close(fd);
}
