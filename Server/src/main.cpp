#include "server.h"

using websocketpp::lib::thread;
using websocketpp::lib::bind;

int main() {
    try {
        Server server_instance;

        // Start a thread to run the processing loop
        thread t(bind(&Server::process_actions, &server_instance));

        // Run the asio loop with the main thread
        server_instance.run(1234);

        t.join();

    }
    catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}
