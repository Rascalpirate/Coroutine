#include <arpa/inet.h>
#include <chrono>
#include <coroutine>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

struct Task
{
    struct promise_type
    {
        Task get_return_object()
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

        void return_void()
        {
        }

        void unhandled_exception()
        {
            std::terminate();
        }
    };
};

struct SendAwaiter
{
    int         fd;
    const char *data;
    size_t      len;

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h)
    {
        int         sfd   = fd;
        const char *sdata = data;
        size_t      slen  = len;
        std::thread(
            [sfd, sdata, slen, h]()
            {
                ::send(sfd, sdata, slen, 0);
                h.resume();
            })
            .detach();
    }

    void await_resume() const noexcept
    {
    }
};

Task run_client()
{
    int         fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(9006);
    ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    const char *msg = "hello from coroutine client\n";
    co_await SendAwaiter {fd, msg, 28};

    char    buf[1024] {};
    ssize_t n = ::recv(fd, buf, sizeof(buf) - 1, 0);
    if (n > 0)
        std::cout << "echo: " << buf;
    ::close(fd);
}

int main()
{
    run_client();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
