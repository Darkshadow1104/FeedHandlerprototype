// feed_handler.cpp
#include "socket.cpp"
#include "parser.cpp"
#include "visualizer.cpp"

int main() {
    MarketDataSocket sock;
    sock.connect("127.0.0.1", 9876);

    Parser parser;


//visualizer thread.

    Visualizer viz(parser.getCache(), parser.getTracker(), 100);

    std::thread ui_thread([&]() {
        viz.run();
    });



    char buffer[65536];

    while (true) {
        int n = sock.receive(buffer, sizeof(buffer));

//		  std::cout << "Received bytes: " << n << "\n";
        if (n > 0) {
//		  std::cout << "Received bytes: " << n << "\n";
            parser.on_data(buffer, n);
        }
    }
    ui_thread.join();
}
