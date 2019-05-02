#include "server.h"

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

// helper variable
template<class T> constexpr bool always_false = std::false_type::value;

Server::Server()
{
    // Initialize Asio Transport
    server_.init_asio();

    // Register handler callbacks
    server_.set_open_handler(bind(&Server::on_open, this, _1));
    server_.set_close_handler(bind(&Server::on_close, this, _1));
    server_.set_message_handler(bind(&Server::on_message, this, _1, _2));
}

void Server::run(uint16_t port)
{
    // listen on specified port
    server_.listen(port);

    // Start the server accept loop
    server_.start_accept();

    // Start the ASIO io_service run loop
    try {
        server_.run();
    }
    catch (const std::exception & e) {
        std::cout << e.what() << std::endl;
    }
}

void Server::on_open(connection_hdl hdl)
{
    {
        lock_guard guard(actionLock_);
        actions_.push(NewPlayer(hdl));
    }
    actionCond_.notify_one();
}

void Server::on_close(connection_hdl hdl)
{
    {
        lock_guard guard(actionLock_);
        actions_.push(PlayerDisconnected(hdl));
    }
    actionCond_.notify_one();
}

void Server::on_message(connection_hdl hdl, WebsocketServer::message_ptr msg)
{
    {
        lock_guard guard(actionLock_);
        actions_.push(Message(hdl, msg));
    }
    actionCond_.notify_one();
}

void Server::process_actions()
{
    while (true) {
        unique_lock lock(actionLock_);

        while (actions_.empty()) {
            actionCond_.wait(lock);
        }

        const auto a = actions_.front();
        actions_.pop();
        lock.unlock();

        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, NewPlayer>) {
                lock_guard guard(connectionsLock_);
                if (game_.started()) {
                    server_.close(arg.hdl, websocketpp::close::status::try_again_later, "Game is full");
                    return;
                }
                connections_.insert(arg.hdl);
                game_.addPlayer();
                if (game_.started()) {
                    const std::string msg("Game started!");
                    for (auto it = connections_.begin(); it != connections_.end(); ++it) {

                        server_.send(*it, msg, websocketpp::frame::opcode::TEXT);
                    }
                }
            }
            else if constexpr (std::is_same_v<T, PlayerDisconnected>) {
                lock_guard guard(connectionsLock_);
                
                if (connections_.count(arg.hdl) == 0)
                    return;
                
                connections_.erase(arg.hdl);
                server_.close(arg.hdl, websocketpp::close::status::normal, "Player disconnected");
            }
            else if constexpr (std::is_same_v<T, Message>) {
                lock_guard guard(connectionsLock_);
                for (auto it = connections_.begin(); it != connections_.end(); ++it) {
                    server_.send(*it, arg.msg);
                }
            }
            else
                static_assert(always_false<T>, "visitor not specified for all actions!");
        }, a);
    }
}
