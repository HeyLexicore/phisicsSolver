# ===============================================
# Makefile for C++ project with SDL2
# ===============================================

# Compiler and flags
CXX       := g++
CXXFLAGS  := -std=c++11 -Wall -Wextra -O2 -g
INCLUDES  := -I/usr/include/SDL2
LIBS      := -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# Automatically find all .cpp files (including subdirectories)
SRC_DIR   := .
SOURCES   := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS   := $(SOURCES:.cpp=.o)
TARGET    := game

# Default target
all: $(TARGET)

# Link the final executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LIBS)

# Compile each .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Rebuild everything
rebuild: clean all

# Phony targets
.PHONY: all clean rebuild
