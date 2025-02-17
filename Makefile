# Paths
RAYLIB_PATH = C:/raylib/raylib

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external -DPLATFORM_DESKTOP
LDFLAGS = -L$(RAYLIB_PATH)/src -L$(RAYLIB_PATH)/src/external

# Full static linking
STATIC_FLAGS = -static -static-libgcc -static-libstdc++ -lpdh -lws2_32 -lpsapi -mwindows

# Raylib linking (assuming static library is available)
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm $(STATIC_FLAGS)

# Project files for Functions_test
SRC_CPU_FQ = CPU_FQ.cpp
OBJ_CPU_FQ = $(SRC_CPU_FQ:.cpp=.o)
EXEC_CPU_FQ = CPU_FQ.exe

# Default targetffunc
all: $(EXEC_CPU_FQ)

# Link the executable for Functions_test with full static linking
$(EXEC_CPU_FQ): $(OBJ_CPU_FQ)
	$(CXX) -o $(EXEC_CPU_FQ) $(OBJ_CPU_FQ) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Compile C++ source files to object files
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Clean up build artifacts
clean:
	@echo Cleaning up build artifacts...
	-@del /q *.o *.exe 2> NUL || rm -f *.o *.exe

.PHONY: all clean