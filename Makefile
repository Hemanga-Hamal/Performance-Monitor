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
SRC_FN = StatsV1.cpp Functions_test.cpp Stats.cpp
OBJ_FN = StatsV1.o Functions_test.o Stats.o
EXEC_FN = Functions_test.exe


# Default targetffunc
all: $(EXEC_GraphicV2)

# Link the executable for Functions_test with full static linking
$(EXEC_FN): $(OBJ_FN)
	$(CXX) -o $(EXEC_FN) $(OBJ_FN) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)



# Compile C++ source files to object files
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Clean up build artifacts
clean:
	@echo Cleaning up build artifacts...
	-@del /q *.o *.exe 2> NUL || rm -f *.o *.exe

.PHONY: all clean