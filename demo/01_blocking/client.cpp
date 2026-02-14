#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    constexpr int kPort = 9001;
    int           fd    = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        std::perror("socket");
        return 1;
    }

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(kPort);
    ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        std::perror("connect");
        return 1;
    }

    const char *msg = "hello from blocking client\n";
    ::send(fd, msg, std::strlen(msg), 0);

    char    buf[1024] {};
    ssize_t n = ::recv(fd, buf, sizeof(buf) - 1, 0);
    if (n > 0)
    {
        std::cout << "echo: " << buf;
    }

    ::close(fd);
}
