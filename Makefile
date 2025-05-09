# Makefile para Projeto-SO
CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDFLAGS = -lglib-2.0

# Diretórios
SRC_DIR = src
SERVER_DIR = $(SRC_DIR)/server
CLIENT_DIR = $(SRC_DIR)/client
BIN_DIR = bin
TARGET_DIR = target

# Arquivos para compilar
SERVER_SRCS = $(wildcard $(SERVER_DIR)/*.c)
CLIENT_SRCS = $(wildcard $(CLIENT_DIR)/*.c)

SERVER_OBJS = $(patsubst $(SERVER_DIR)/%.c, $(TARGET_DIR)/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst $(CLIENT_DIR)/%.c, $(TARGET_DIR)/%.o, $(CLIENT_SRCS))

SERVER_TARGET = $(BIN_DIR)/dserver
CLIENT_TARGET = $(BIN_DIR)/dclient

# Alvos principais
all: dirs server client

# Criar diretórios necessários
dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(TARGET_DIR)

# Regra para servidor
server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Regra para cliente
client: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Regras genéricas para compilação
$(TARGET_DIR)/%.o: $(SERVER_DIR)/%.c
	$(CC) -c $(CFLAGS) -I$(SERVER_DIR) -I$(CLIENT_DIR) $< -o $@

$(TARGET_DIR)/%.o: $(CLIENT_DIR)/%.c
	$(CC) -c $(CFLAGS) -I$(SERVER_DIR) -I$(CLIENT_DIR) $< -o $@

# Limpar arquivos temporários
clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS)
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
	rm -rf $(BIN_DIR)
	rm -rf $(TARGET_DIR)

# Limpar e recompilar tudo
rebuild: clean all

# Executar o servidor
run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

.PHONY: all dirs server client clean rebuild run-server
