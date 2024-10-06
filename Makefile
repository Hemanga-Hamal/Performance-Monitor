# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external -DPLATFORM_DESKTOP
LDFLAGS = -L$(RAYLIB_PATH)/src -L$(RAYLIB_PATH)/src/external

# Full static linking
STATIC_FLAGS = -static -static-libgcc -static-libstdc++ -lpdh

# Raylib linking (assuming static library is available)
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm $(STATIC_FLAGS)

# Paths
RAYLIB_PATH = C:/raylib/raylib

# Project files for Functions_test
SRC_FN =  stats.cpp main.cpp
OBJ_FN = $(SRC_FN:.cpp=.o)
EXEC_FN = Functions_test.exe

# Default target
all: $(EXEC_FN) 

# Link the executable for PL_test with full static linking -> Player
$(EXEC_FN): $(OBJ_FN)
	$(CXX) -o $(EXEC_FN) $(OBJ_FN) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Compile C++ source files to object files
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Clean up build artifacts
clean:
ifeq ($(OS),Windows_NT)
	del /q *.o *.exe
else
	rm -f *.o *.exe
endif

.PHONY: all clean
