cmake_minimum_required(VERSION 3.10)
project(HFTFeedHandler)

set(CMAKE_CXX_STANDARD 20)

add_executable(server src/server/main.cpp 
	src/server/exchange_simulator.cpp)
add_executable(client src/client/feed_handler.cpp)
