#include "../include/Server.h"

int main(int argc, char* argv[]) {
    Server server(argv[1], std::stoi(argv[2]));
    server.run();
}
