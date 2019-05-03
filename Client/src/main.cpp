#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

// This header pulls in the WebSocket++ abstracted thread support that will
// select between boost::thread and std::thread based on how the build system
// is configured.
#include <websocketpp/common/thread.hpp>
#include <google/protobuf/util/json_util.h>
#include "protocol.pb.h"


/**
 * Define a semi-cross platform helper method that waits/sleeps for a bit.
 */
void wait_a_bit() {
#ifdef WIN32
    Sleep(5);
#else
    sleep(3);
#endif
}

/**
 * The telemetry client connects to a WebSocket server and sends a message every
 * second containing an integer count. This example can be used as the basis for
 * programs where a client connects and pushes data for logging, stress/load
 * testing, etc.
 */
class Client {
public:
    using WebsocketClient = websocketpp::client<websocketpp::config::asio_client> ;
    using ScopedLock = websocketpp::lib::lock_guard<websocketpp::lib::mutex>;

    Client() : isOpen_(false), isDone_(false) {
        // set up access channels to only log interesting things
        client_.clear_access_channels(websocketpp::log::alevel::all);
        client_.set_access_channels(websocketpp::log::alevel::connect);
        client_.set_access_channels(websocketpp::log::alevel::disconnect);
        client_.set_access_channels(websocketpp::log::alevel::app);

        // Initialize the Asio transport policy
        client_.init_asio();

        // Bind the handlers we are using
        using websocketpp::lib::placeholders::_1;
        using websocketpp::lib::placeholders::_2;
        using websocketpp::lib::bind;
        client_.set_open_handler(bind(&Client::onOpen, this, _1));
        client_.set_close_handler(bind(&Client::onClose, this, _1));
        client_.set_message_handler(bind(&Client::onMessage, this, _1, _2));
        client_.set_fail_handler(bind(&Client::onFail, this, _1));
    }

    // This method will block until the connection is complete
    void run(const std::string & uri) {
        // Create a new connection to the given URI
        websocketpp::lib::error_code ec;
        WebsocketClient::connection_ptr con = client_.get_connection(uri, ec);
        if (ec) {
            client_.get_alog().write(websocketpp::log::alevel::app,
                "Get Connection Error: " + ec.message());
            return;
        }

        // Grab a handle for this connection so we can talk to it in a thread
        // safe manor after the event loop starts.
        connection_ = con->get_handle();

        // Queue the connection. No DNS queries or network connections will be
        // made until the io_service event loop is run.
        client_.connect(con);

        // Create a thread to run the ASIO io_service event loop
        websocketpp::lib::thread asioThread(&WebsocketClient::run, &client_);

        // Create a thread to run the commands thread
        websocketpp::lib::thread commandThread(&Client::commandsLoop, this);

        asioThread.join();
        commandThread.join();
    }

    // The open handler will signal that we are ready to start sending telemetry
    void onOpen(websocketpp::connection_hdl) {
        client_.get_alog().write(websocketpp::log::alevel::app,
            "Connection opened!");

        ScopedLock guard(lock_);
        isOpen_ = true;
    }

    // The close handler will signal that we should stop sending telemetry
    void onClose(websocketpp::connection_hdl) {
        client_.get_alog().write(websocketpp::log::alevel::app,
            "Connection closed!");

        ScopedLock guard(lock_);
        isDone_ = true;
    }

    void onMessage(websocketpp::connection_hdl hdl, WebsocketClient::message_ptr msg) {
        websocketpp::lib::error_code ec;
        GameState state;
        if (state.ParseFromString(msg->get_payload()))
        {
            std::stringstream ss;
            //google::protobuf::util::MessageToJsonString(state, &str);
            //std::cout << str;
            ss << "get game state for player: " << state.player();
            ScopedLock guard(lock_);
            client_.get_alog().write(websocketpp::log::alevel::app, ss.str());
        }
    }

    // The fail handler will signal that we should stop sending telemetry
    void onFail(websocketpp::connection_hdl) {
        client_.get_alog().write(websocketpp::log::alevel::app,
            "Connection failed!");

        ScopedLock guard(lock_);
        isDone_ = true;
    }

    void commandsLoop() {
        std::stringstream val;
        websocketpp::lib::error_code ec;

        while (true) {
            bool wait = false;

            {
                ScopedLock guard(lock_);
                // If the connection has been closed, stop generating telemetry
                if (isDone_) { break; }

                // If the connection hasn't been opened yet wait a bit and retry
                if (!isOpen_) {
                    wait = true;
                }
            }

            if (wait) {
                wait_a_bit();
                continue;
            }

            client_.get_alog().write(websocketpp::log::alevel::app, "send end turn");

            Command cmd;
            cmd.set_allocated_end_turn(new EndTurn()); // deallocate automaticly
            client_.send(connection_, cmd.SerializeAsString(), websocketpp::frame::opcode::BINARY, ec);

            if (ec) {
                client_.get_alog().write(websocketpp::log::alevel::app,
                    "Send Error: " + ec.message());
                break;
            }

            wait_a_bit();
        }
    }
private:
    WebsocketClient client_;
    websocketpp::connection_hdl connection_;
    websocketpp::lib::mutex lock_;
    bool isOpen_;
    bool isDone_;
};

int main(int argc, char* argv[]) {
    Client c;

    std::string uri = "ws://localhost:1234";

    if (argc == 2) {
        uri = argv[1];
    }

    c.run(uri);
}

