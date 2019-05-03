#pragma once
#include <map>
#include <variant>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include "game.h"
#include "protocol.pb.h"

/*! Base server notification, contains connection handler */
struct Notification {
    Notification(websocketpp::connection_hdl conn) : connection(conn) {}
    websocketpp::connection_hdl connection;
};

/*! Notification to add new player/connection */
struct AddPlayer : public Notification {
    using Notification::Notification;
};

/*! Notification to remove player/connection */
struct PlayerDisconnected : public Notification {
    using Notification::Notification;
};

/*! Notification-command recieved from player */
struct CommandNotification : public Notification {
    CommandNotification(websocketpp::connection_hdl h, const Command& cmd) : Notification(h), command(cmd) {}
    Command command;
};

class Server {
public:
    Server();

    /*! Run server, listening specified port */
    void run(uint16_t port);

private:
    using WebsocketServer = websocketpp::server<websocketpp::config::asio>;
    using Connection = websocketpp::connection_hdl;
    using ConnectionsContainer = std::map<Connection, int, std::owner_less<Connection>>;
    using ServerMsg = WebsocketServer::message_ptr;
    using Mutex = websocketpp::lib::mutex;
    using ConditionVar = websocketpp::lib::condition_variable;

    /*! websocketpp server instance */
    WebsocketServer server_;

    /*! All active server connections: connection -> playerId */
    ConnectionsContainer connections_;

    /*! Mutex to sync connections_ */
    // Mutex connectionsLock_;

    /*! Queue consumer of server notifications */
    std::queue<std::variant<
        AddPlayer,
        PlayerDisconnected,
        CommandNotification>> notifications_;

    /*! Mutex to sync notifications_ */
    Mutex notificationsLock_;

    /*! Condition variable to process notifications queue */
    ConditionVar notificationCond_;

    /*! Game instance */
    game::Game game_;

    /*! Open new connection handler */
    void onOpen(Connection conn);

    /*! Close connection handler */
    void onClose(Connection conn);

    /*! Recieve message handler */
    void onMessage(Connection conn, ServerMsg msg);

    /*! Handles notifications from server in separate thread */
    void processNotifications();
};
