// src/server/main.cpp
#include <iostream>
#include <signal.h>
#include "exchange_simulator.h"
// forward declare (or include header if you have one)

int main() {
  signal(SIGPIPE, SIG_IGN);
    	std::cout << "Starting Exchange Simulator...\n";

    ExchangeSimulator server(9876, 100);
    server.run();

    return 0;
}
