#include <iostream>
#include "game_manager.h"
#include "session.h"
#include "game_manager.h"

/*
void Session::run()
{
    // Accept the websocket handshake
    ws_.async_accept(
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &Session::on_accept,
                shared_from_this(),
                std::placeholders::_1)));
}

void Session::on_accept(boost::system::error_code ec)
{
    if (ec)
        return fail(ec, "accept");

    manager_->subscribe(std::bind(&Session::sendState, shared_from_this(), 1));

    // Read a message
    do_read();
}

void Session::do_read()
{
    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &Session::on_read,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2)));
}

void Session::on_read(boost::system::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed)
        return;

    if (ec)
        fail(ec, "read");

    std::cout << boost::beast::buffers_to_string(buffer_.data()) << std::endl;

    manager_->consumeCommand();

    //sendState(42);

    // Echo the message
    //ws_.text(ws_.got_text());
    //ws_.async_write(
    //    buffer_.data(),
    //    boost::asio::bind_executor(
    //        strand_,
    //        std::bind(
    //            &Session::on_write,
    //            shared_from_this(),
    //            std::placeholders::_1,
    //            std::placeholders::_2)));
}

void Session::on_write(boost::system::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "write");

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Do another read
    do_read();
}
*/
