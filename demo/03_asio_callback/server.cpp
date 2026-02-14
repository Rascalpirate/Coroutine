#include <array>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(tcp::socket socket) : socket_(std::move(socket))
    {
    }

    void start()
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self = shared_from_this();
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [this, self](boost::system::error_code ec, std::size_t n)
            {
                if (ec)
                    return;
                do_write(n);
            });
    }

    void do_write(std::size_t n)
    {
        auto self = shared_from_this();
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(buffer_.data(), n),
            [this, self](boost::system::error_code ec, std::size_t)
            {
                if (ec)
                    return;
                do_read();
            });
    }

    tcp::socket            socket_;
    std::array<char, 1024> buffer_ {};
};

int main()
{
    boost::asio::io_context io;
    tcp::acceptor           acceptor(io, tcp::endpoint(tcp::v4(), 9003));

    std::function<void()> do_accept;
    do_accept = [&]()
    {
        acceptor.async_accept(
            [&](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                    std::make_shared<Session>(std::move(socket))->start();
                do_accept();
            });
    };

    do_accept();
    std::cout << "[asio callback server] listen on 9003\n";
    io.run();
}
