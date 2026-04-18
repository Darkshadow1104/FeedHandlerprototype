#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>

int main() {
    std::ofstream out("md_in.txt");

    if (!out.is_open()) {
        std::cerr << "Failed to open md_in.txt\n";
        return 1;
    }

    // Random generators
    std::mt19937 gen(42); // fixed seed → reproducible

    std::uniform_int_distribution<> ticker_dist(1, 10);
    std::uniform_real_distribution<> price_dist(100.0, 200.0);
    std::uniform_int_distribution<> qty_dist(10, 50);

    const int NUM_LINES = 100000; // adjust for load testing

    for (int i = 0; i < NUM_LINES; i++) {
        int ticker = ticker_dist(gen);
        double price = price_dist(gen);
        int qty = qty_dist(gen);

        out << ticker << ","
            << std::fixed << std::setprecision(2) << price << ","
            << qty << "\n";
    }

    out.close();

    std::cout << "Generated md_in.txt with " << NUM_LINES << " lines\n";
    return 0;
}
