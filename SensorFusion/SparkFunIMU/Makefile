CC=gcc 

CFLAGS   := -Iinclude -pipe -pedantic -g -Wall -Wextra -O3 -ffast-math
LDFLAGS  := 
LDLIBS   := -lm

SRC_DIR:=src
BIN_DIR:=bin
OBJ_DIR:=bin/obj

EXE:=$(BIN_DIR)/IMU
SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
	

.PHONY: all
all: $(EXE)
	@echo "Linking DONE"
$(EXE): $(OBJ)
	@echo "Linking..."
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@
	@echo "Compiling..."
.PHONY: clean
clean:
	@echo "Cleaning..."
	rm  -r $(BIN_DIR)/*
.PHONY: start
start:
	@echo "Starting..."
	./$(EXE)