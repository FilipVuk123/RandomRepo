
CXX=g++ 
CC=gcc
CFLAGS   := -Iinclude -pipe -Wall -pedantic -g -Wextra 
CXXFLAGS := -Iinclude \
			-I/home/filip/opencv-4.x/include \
			-I/home/filip/opencv-4.x/modules/core/include \
			-I/home/filip/opencv-4.x/modules/video/include \
			-I/home/filip/opencv-4.x/modules/stitching/include \
			-I/home/filip/opencv-4.x/modules/ml/include \
			-I/home/filip/opencv-4.x/modules/photo/include \
			-I/home/filip/opencv-4.x/modules/flann/include \
			-I/home/filip/opencv-4.x/modules/features2d/include \
			-I/home/filip/opencv-4.x/modules/calib3d/include \
			-I/home/filip/opencv-4.x/modules/dnn/include \
			-I/home/filip/opencv-4.x/modules/core/include \
			-I/home/filip/opencv-4.x/modules/imgproc/include \
			-I/home/filip/opencv-4.x/modules/objdetect/include \
			-I/home/filip/opencv-4.x/modules/videoio/include \
			-I/home/filip/opencv-4.x/modules/imgcodecs/include \
			-I/home/filip/opencv-4.x/modules/highgui/include -Wall -pipe -g -Wextra -pg \
			-I/home/filip/opencv_build \

LDFLAGS  := `pkg-config --cflags opencv` -L/home/filip/opencv_build/lib
LDLIBS   := -ldl -lm -lglfw -L/home/filip/opencv_build/lib `pkg-config --libs opencv` -L/home/filip/opencv_build/lib -lopencv_dnn -lavcodec -lavformat -lavutil
 
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