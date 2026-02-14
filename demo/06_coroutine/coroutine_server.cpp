#include <arpa/inet.h>
#include <coroutine>
#include <exception>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

struct FireAndForget
{
    struct promise_type
    {
        FireAndForget get_return_object()
        {
            return {};
        }

        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }

        std::suspend_never final_suspend() noexcept
        {
            return {};
        }

        void unhandled_exception()
        {
            std::terminate();
        }

        void return_void()
        {
        }
    };
};

struct RecvAwaiter
{
    int     fd;
    char   *buf;
    size_t  len;
    ssize_t result {0};

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h)
    {
        std::thread(
            [this, h]() mutable
            {
                result = ::recv(fd, buf, len, 0);
                h.resume();
            })
            .detach();
    }

    ssize_t await_resume() const noexcept
    {
        return result;
    }
};

FireAndForget echo_session(int conn)
{
    char buf[1024];
    while (true)
    {
        ssize_t n = co_await RecvAwaiter {conn, buf, sizeof(buf)};
        if (n <= 0)
            break;
        ::send(conn, buf, static_cast<size_t>(n), 0);
    }
    ::close(conn);
}

int main()
{
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    int opt       = 1;
    ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(9006);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ::bind(listen_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    ::listen(listen_fd, 64);

    std::cout << "[coroutine server] listen on 9006\n";
    while (true)
    {
        int conn = ::accept(listen_fd, nullptr, nullptr);
        if (conn >= 0)
            echo_session(conn);
    }
}
