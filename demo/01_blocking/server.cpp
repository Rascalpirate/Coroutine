#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    constexpr int kPort     = 9001;
    int           listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        std::perror("socket");
        return 1;
    }

    int opt = 1;
    ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(kPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(listen_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))
        < 0)
    {
        std::perror("bind");
        return 1;
    }
    if (::listen(listen_fd, 128) < 0)
    {
        std::perror("listen");
        return 1;
    }

    std::cout << "[blocking server] listen on " << kPort << '\n';
    while (true)
    {
        int conn_fd = ::accept(listen_fd, nullptr, nullptr);
        if (conn_fd < 0)
        {
            std::perror("accept");
            continue;
        }

        char buf[1024];
        while (true)
        {
            ssize_t n = ::recv(conn_fd, buf, sizeof(buf), 0);
            if (n > 0)
            {
                ::send(conn_fd, buf, static_cast<size_t>(n), 0); // echo
            }
            else
            {
                break;
            }
        }
        ::close(conn_fd);
    }
}
