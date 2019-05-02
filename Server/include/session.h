#pragma once
/*
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

class GameManager;

// Report a failure
inline void fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
class Session : public std::enable_shared_from_this<Session>
{
    websocket::stream<tcp::socket> ws_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::beast::multi_buffer buffer_;
    std::shared_ptr<GameManager> manager_;
    std::string text_;

public:
    // Take ownership of the socket
    explicit Session(tcp::socket socket, std::shared_ptr<GameManager> manager) :
        ws_(std::move(socket)), strand_(ws_.get_executor()), manager_(manager)
    {
        //manager_->subscribe(std::bind(&Session::sendState, shared_from_this(), 42));
    }

    void sendState(int i) {
        ws_.text(true);

        // clear buffer
        //buffer_.consume(buffer_.size());

        //boost::beast::buffers_to_string
        text_ = std::to_string(i);

        ws_.write(boost::asio::buffer(text_));

        do_read();

        //ws_.async_write(
        //    boost::asio::buffer(text_),
        //    boost::asio::bind_executor(
        //        strand_,
        //        std::bind(
        //            &Session::on_write,
        //            shared_from_this(),
        //            std::placeholders::_1,
        //            std::placeholders::_2)));
    }

    // Start the asynchronous operation
    void run();

    void on_accept(boost::system::error_code ec);

    void do_read();
    void on_read(boost::system::error_code ec, std::size_t bytes_transferred);

    void on_write(boost::system::error_code ec, std::size_t bytes_transferred);


};
*/