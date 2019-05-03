#include "server.h"

using websocketpp::lib::thread;
using websocketpp::lib::bind;

int main() {
    try {
        Server server_instance;
        server_instance.run(1234);
    }
    catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}
