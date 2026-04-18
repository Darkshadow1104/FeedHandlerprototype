CXX = g++
CXXFLAGS = -O2 -std=c++17 -pthread -Iinclude -Wall -Wextra

SRC_DIR = src
BIN_DIR = bin

SERVER_SRCS = $(SRC_DIR)/Server.cpp $(SRC_DIR)/server_main.cpp
CLIENT_SRCS = $(SRC_DIR)/Client.cpp $(SRC_DIR)/client_main.cpp

SERVER_BIN = $(BIN_DIR)/Server
CLIENT_BIN = $(BIN_DIR)/Client

# Default target
all: dirs server client

# Create bin directory
dirs:
	mkdir -p $(BIN_DIR)

# Build server
server:
	$(CXX) $(CXXFLAGS) $(SERVER_SRCS) -o $(SERVER_BIN)

# Build client
client:
	$(CXX) $(CXXFLAGS) $(CLIENT_SRCS) -o $(CLIENT_BIN)

# Clean
clean:
	rm -rf $(BIN_DIR)

# Run helpers
run-server:
	./$(SERVER_BIN) 127.0.0.1 6666 < md_in.txt

run-client:
	./$(CLIENT_BIN) 127.0.0.1 6666

GEN_SRC = src/gen_input.cpp
GEN_BIN = $(BIN_DIR)/gen_input

gen:
	$(CXX) $(CXXFLAGS) $(GEN_SRC) -o $(GEN_BIN)

run-gen:
	./$(GEN_BIN)
# Phony targets
.PHONY: all clean dirs server client run-server run-client
