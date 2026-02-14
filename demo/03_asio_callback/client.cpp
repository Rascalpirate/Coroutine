#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>

using boost::asio::ip::tcp;

int main()
{
    boost::asio::io_context io;
    auto                    socket = std::make_shared<tcp::socket>(io);
    auto                    message
        = std::make_shared<std::string>("hello from asio callback client\n");
    auto buffer = std::make_shared<std::array<char, 1024>>();

    tcp::resolver resolver(io);
    auto          endpoints = resolver.resolve("127.0.0.1", "9003");

    boost::asio::async_connect(
        *socket,
        endpoints,
        [socket,
         message,
         buffer](boost::system::error_code ec, const tcp::endpoint &)
        {
            if (ec)
                return;
            boost::asio::async_write(
                *socket,
                boost::asio::buffer(*message),
                [socket, buffer](boost::system::error_code ec2, std::size_t)
                {
                    if (ec2)
                        return;
                    socket->async_read_some(
                        boost::asio::buffer(*buffer),
                        [buffer](boost::system::error_code ec3, std::size_t n)
                        {
                            if (!ec3)
                                std::cout << "echo: "
                                          << std::string(buffer->data(), n);
                        });
                });
        });

    io.run();
}
