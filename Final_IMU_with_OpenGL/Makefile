CC=gcc 

CFLAGS   := -Iinclude -pipe -pedantic -g -Wall -Wextra -O3 -ffast-math
LDFLAGS  := 
LDLIBS   := -lm -ldl -lglfw -lavutil -lavformat -lavcodec -pthread

SRC_DIR:=src
BIN_DIR:=bin
OBJ_DIR:=bin/obj

EXE:=$(BIN_DIR)/IMU_parser
SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))


.PHONY: all

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@
.PHONY: clean
clean:
	rm  -r $(BIN_DIR)/*
.PHONY: start
start:
	./$(EXE)
