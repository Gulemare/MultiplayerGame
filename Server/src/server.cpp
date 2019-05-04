#include <type_traits>
#include <iostream>
#include <google/protobuf/util/json_util.h>

#include "server.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;

/*! Helper variable, false for any type */
template<class T> constexpr bool always_false = std::false_type::value;

Server::Server()
{
    // Initialize Asio Transport
    server_.init_asio();

    server_.clear_access_channels(websocketpp::log::alevel::frame_payload);
    server_.clear_access_channels(websocketpp::log::alevel::frame_header);

    // Register handler callbacks
    server_.set_open_handler(bind(&Server::onOpen, this, _1));
    server_.set_close_handler(bind(&Server::onClose, this, _1));
    server_.set_message_handler(bind(&Server::onMessage, this, _1, _2));
}

void Server::run(uint16_t port)
{
    // listen on specified port
    server_.listen(port);

    // Start the server accept loop
    server_.start_accept();

    // Start a separate thread to run the processing loop
    thread t(bind(&Server::processNotifications, this));

    // Start the ASIO io_service run loop
    try {
        server_.run();
        t.join();
    }
    catch (const std::exception & e) {
        std::cout << e.what() << std::endl;
    }
}

void Server::onOpen(Connection conn)
{
    {
        lock_guard guard(notificationsLock_);
        notifications_.push(AddPlayer(conn));
    }
    notificationCond_.notify_one();
}

void Server::onClose(Connection conn)
{
    {
        lock_guard guard(notificationsLock_);
        notifications_.push(PlayerDisconnected(conn));
    }
    notificationCond_.notify_one();
}

void Server::onMessage(Connection conn, ServerMsg msg)
{
    {
        Command cmd;
        if (!cmd.ParseFromString(msg->get_payload()))
            return;

        lock_guard guard(notificationsLock_);
        notifications_.push(CommandNotification(conn, cmd));
    }
    notificationCond_.notify_one();
}

void Server::processNotifications()
{
    const auto sendGameState = [this]() {
        auto state = game_.getState();
        for (auto&&[connection, playerId] : connections_) {
            state.set_player(playerId);
            state.set_team(game_.getPlayerTeam(playerId));
            server_.send(connection, state.SerializeAsString(), websocketpp::frame::opcode::BINARY);
        }
    };

    while (true) {
        unique_lock lock(notificationsLock_);

        while (notifications_.empty()) {
            notificationCond_.wait(lock);
        }

        const auto notification = notifications_.front();
        notifications_.pop();
        lock.unlock();

        // Handle notification
        std::visit([this, sendGameState](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, AddPlayer>) {
                // Add players to game if can, else if game is full - close new connections
                //lock_guard guard(connectionsLock_);
                if (game_.started()) {
                    server_.close(arg.connection, websocketpp::close::status::try_again_later, "Game is full");
                    return;
                }
                const auto playerId = game_.addPlayer();
                connections_.insert({ arg.connection, playerId });
                
                if (game_.started())
                    sendGameState();
            }
            else if constexpr (std::is_same_v<T, PlayerDisconnected>) {
                // Disconnect all other players and restart a game
                //lock_guard guard(connectionsLock_);

                if (connections_.count(arg.connection) == 0)
                    return;

                connections_.erase(arg.connection);
                for (auto&&[connection, playerNum] : connections_) {
                    server_.close(connection, websocketpp::close::status::normal, "Player disconnected");
                }
                connections_.clear();
                game_.restart(2);
            }
            else if constexpr (std::is_same_v<T, CommandNotification>) {
                // Handle command from players, if command is valid - update game and send new state
                //lock_guard guard(connectionsLock_);
                server_.get_alog().write(websocketpp::log::alevel::app, "read message");
                if (!game_.started())
                    return;

                const auto playerId = connections_.at(arg.connection);
                if (!game_.consumeCommand(playerId, arg.command))
                    return;

                std::string str;
                google::protobuf::util::MessageToJsonString(arg.command, &str);
                std::stringstream ss;
                ss << playerId << " do command: " << str;
                server_.get_alog().write(websocketpp::log::alevel::app, ss.str());

                sendGameState();
            }
            else
                static_assert(always_false<T>, "visitor not specified for all actions!");
        }, notification);
    }
}
