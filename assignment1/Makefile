# Makefile for assignment1 (MSYS2 UCRT64 toolchain on Windows)
#
# Usage (from an MSYS2 UCRT64 shell, or with C:\msys64\ucrt64\bin and
# C:\msys64\usr\bin on PATH):
#   make        - build bin/main.exe and copy the DLL + shader next to it
#   make run    - build and run (from bin/, so "res/shaders/..." resolves)
#   make clean  - remove build output

ROOT    := ..
SRC_DIR := $(ROOT)/src
BIN     := bin
OBJ     := $(BIN)/obj

CXX      := g++
CC       := gcc
INCLUDES := -I$(ROOT)/include -I$(SRC_DIR)
CXXFLAGS := -std=c++17 -Wall -g -fdiagnostics-color=always $(INCLUDES)
CFLAGS   := -std=c11 -Wall -g $(INCLUDES)
LDFLAGS  := -L$(ROOT)/lib/windows
LDLIBS   := -lglfw3dll -lopengl32

# Shared engine sources from ../src (excluding its own main.cpp) + this assignment's main.cpp
LIB_SRCS := $(filter-out $(SRC_DIR)/main.cpp,$(wildcard $(SRC_DIR)/*.cpp))
LIB_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ)/%.o,$(LIB_SRCS)) $(OBJ)/glad.o
OBJS     := $(OBJ)/main.o $(LIB_OBJS)
TARGET   := $(BIN)/main.exe

RUNTIME  := $(BIN)/glfw3.dll $(BIN)/res/shaders/basic.shader

.PHONY: all run clean

all: $(TARGET) $(RUNTIME)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ)/main.o: main.cpp | $(OBJ)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC_DIR)/%.cpp | $(OBJ)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/glad.o: $(SRC_DIR)/glad.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN)/glfw3.dll: $(ROOT)/lib/windows/glfw3.dll | $(BIN)
	cp $< $@

$(BIN)/res/shaders/basic.shader: basic.shader
	mkdir -p $(dir $@)
	cp $< $@

$(OBJ) $(BIN):
	mkdir -p $@

run: all
	cd $(BIN) && ./main.exe

clean:
	rm -rf $(BIN)
