#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include <iostream>
#include <set>
#include <variant>
#include <type_traits>
#include "game.h"

struct ActionType {
    ActionType(websocketpp::connection_hdl h) : hdl(h) {}
    websocketpp::connection_hdl hdl;
};

struct NewPlayer : public ActionType {
    using ActionType::ActionType;
};

struct PlayerDisconnected : public ActionType {
    using ActionType::ActionType;
};

template <typename T>
struct PlayerMessage : public ActionType {
    PlayerMessage(websocketpp::connection_hdl h, T m) : ActionType(h), msg(m) {}
    T msg;
};

/* on_open insert connection_hdl into channel
 * on_close remove connection_hdl from channel
 * on_message queue send to all channels
 */
class Server {
public:
    using WebsocketServer = websocketpp::server<websocketpp::config::asio>;

    Server();

    void run(uint16_t port);
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, WebsocketServer::message_ptr msg);

    void process_actions();

private:
    
    WebsocketServer server_;

    using ConnectionsContainer = std::set<websocketpp::connection_hdl,
        std::owner_less<websocketpp::connection_hdl> >;

    ConnectionsContainer connections_;
    websocketpp::lib::mutex connectionsLock_;
    
    using MessageAction = PlayerMessage<WebsocketServer::message_ptr>;
    using Action = std::variant<NewPlayer, PlayerDisconnected, MessageAction>;
    std::queue<Action> actions_;
    websocketpp::lib::mutex actionLock_;
    websocketpp::lib::condition_variable actionCond_;

    game::Game game_;
};