#include "Client.h"
#include<iostream>
int main(int argc, char* argv[]) {
    Client client(argv[1], std::stoi(argv[2]));
    client.run();
}
