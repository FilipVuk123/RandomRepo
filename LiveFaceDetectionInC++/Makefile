
CXX=g++ 
CC=gcc
CFLAGS := -Iinclude  -pipe -Wall -pedantic -g -Wextra
CXXFLAGS := -Iinclude -I  -Wall -O3 -ffast-math -Iinclude  -pipe -g -Wextra -pg
LDFLAGS  := -L/home/filip/open_cv_build/lib/ `pkg-config --cflags opencv`
LDLIBS   := -ldl -lm -lglfw `pkg-config --libs opencv`

SRC_DIR:=src
BIN_DIR:=bin
OBJ_DIR:=bin/obj

EXE:=$(BIN_DIR)/LiveFaceRecognition

SRCCXX:=$(wildcard $(SRC_DIR)/*.cpp)
SRCC:=$(wildcard $(SRC_DIR)/*.c)

OBJ:=$(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCCXX))
OBJ+=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCC))

.PHONY: all

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@
.PHONY: clean
clean:
	rm  -r bin/*