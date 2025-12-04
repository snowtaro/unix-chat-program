CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
LDFLAGS = -lpthread -lncursesw
INCLUDES = -Iinclude

SRC_DIR = src
BIN_DIR = bin

SERVER_SRC = $(SRC_DIR)/chat_server.c
CLIENT_SRC = $(SRC_DIR)/chat_client.c

SERVER_BIN = $(BIN_DIR)/server
CLIENT_BIN = $(BIN_DIR)/client

all: $(BIN_DIR) $(SERVER_BIN) $(CLIENT_BIN)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(SERVER_BIN): $(SERVER_SRC) include/chat_common.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(SERVER_SRC) $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_SRC) include/chat_common.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(CLIENT_SRC) $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
