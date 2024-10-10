# Paths
RAYLIB_PATH = C:/raylib/raylib

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external -DPLATFORM_DESKTOP
LDFLAGS = -L$(RAYLIB_PATH)/src -L$(RAYLIB_PATH)/src/external

# Full static linking
STATIC_FLAGS = -static -static-libgcc -static-libstdc++ -lpdh

# Raylib linking (assuming static library is available)
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm $(STATIC_FLAGS)

# Project files for Functions_test
SRC_FN = stats.cpp Functions_test.cpp
OBJ_FN = stats.o Functions_test.o
EXEC_FN = Functions_test.exe

# Project files for Graphicv0
SRC_Graph = Graphicv0.cpp
OBJ_Graph = Graphicv0.o
EXEC_Graph = Graphicv0.exe

# Project files for Graphic_Gauge
SRC_Graph_Gauge = Graphic_Gauge.cpp Gauge.cpp
OBJ_Graph_Gauge = Graphic_Gauge.o Gauge.o
EXEC_Graph_Gauge = Graphic_Gauge.exe

# Project files for Graphic_Bar
SRC_Graph_Bar = Graphic_Bar.cpp Bar.cpp
OBJ_Graph_Bar = Graphic_Bar.o Bar.o
EXEC_Graph_Bar = Graphic_Bar.exe

# Project files for Graphic_All (updated to match actual filename)
SRC_Graph_ALL = Graphic_All.cpp Gauge.cpp Bar.cpp stats.cpp
OBJ_Graph_ALL = Graphic_All.o Gauge.o Bar.o stats.o
EXEC_Graph_ALL = Graphic_All.exe

# Default target
all: $(EXEC_FN) $(EXEC_Graph) $(EXEC_Graph_Gauge) $(EXEC_Graph_Bar) $(EXEC_Graph_ALL)

# Link the executable for Functions_test with full static linking
$(EXEC_FN): $(OBJ_FN)
	$(CXX) -o $(EXEC_FN) $(OBJ_FN) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Graphicv0 with full static linking
$(EXEC_Graph): $(OBJ_Graph)
	$(CXX) -o $(EXEC_Graph) $(OBJ_Graph) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Graphic_Gauge with full static linking
$(EXEC_Graph_Gauge): $(OBJ_Graph_Gauge)
	$(CXX) -o $(EXEC_Graph_Gauge) $(OBJ_Graph_Gauge) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Graphic_Bar with full static linking
$(EXEC_Graph_Bar): $(OBJ_Graph_Bar)
	$(CXX) -o $(EXEC_Graph_Bar) $(OBJ_Graph_Bar) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Graphic_All with full static linking
$(EXEC_Graph_ALL): $(OBJ_Graph_ALL)
	$(CXX) -o $(EXEC_Graph_ALL) $(OBJ_Graph_ALL) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

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