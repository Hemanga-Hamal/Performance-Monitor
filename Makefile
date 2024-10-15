# Paths
RAYLIB_PATH = C:/raylib/raylib

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external -DPLATFORM_DESKTOP
LDFLAGS = -L$(RAYLIB_PATH)/src -L$(RAYLIB_PATH)/src/external

# Full static linking
STATIC_FLAGS = -static -static-libgcc -static-libstdc++ -lpdh - mwindows

# Raylib linking (assuming static library is available)
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm $(STATIC_FLAGS)

# Project files for Functions_test
SRC_FN = stats.cpp Functions_test.cpp
OBJ_FN = stats.o Functions_test.o
EXEC_FN = Functions_test.exe

# Project files for Graphicv0
SRC_GraphicV0 = Graphicv0.cpp Bar.cpp Gauge.cpp
OBJ_GraphicV0 = Graphicv0.o Bar.o Gauge.o
EXEC_GraphicV0 = Graphicv0.exe

# Default target
all: $(EXEC_FN) $(EXEC_GraphicV0)

# Link the executable for Functions_test with full static linking
$(EXEC_FN): $(OBJ_FN)
	$(CXX) -o $(EXEC_FN) $(OBJ_FN) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Graphicv0 with full static linking
$(EXEC_GraphicV0): $(OBJ_GraphicV0)
	$(CXX) -o $(EXEC_GraphicV0) $(OBJ_GraphicV0) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Compile C++ source files to object files
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Clean up build artifacts
clean:
	@echo Cleaning up build artifacts...
	-@del /q *.o *.exe 2> NUL || rm -f *.o *.exe

.PHONY: all clean