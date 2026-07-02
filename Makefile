CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Isrc -Iraylib/src
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
  LDFLAGS = -Lraylib/build/raylib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
  LDFLAGS = -Lraylib/build/raylib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lXcursor -lXinerama -lXrandr -lXi
endif

SRC = main.cpp src/debug.cpp src/game.cpp src/input.cpp src/objects.cpp src/physics.cpp src/player.cpp src/skybox.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
