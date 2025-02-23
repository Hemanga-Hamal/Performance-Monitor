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

# Project files for Graphicv2
SRC_GraphicV2 = Graphicv2.cpp BarV1.cpp GaugeV1.cpp StatsV1.cpp
OBJ_GraphicV2 = Graphicv2.o BarV1.o GaugeV1.o StatsV1.o
EXEC_GraphicV2 = Graphicv2.exe

# Project files for Graphicv2T
SRC_GraphicV2T = Graphicv2T.cpp BarV1.cpp GaugeV1.cpp StatsV1.cpp
OBJ_GraphicV2T = Graphicv2T.o BarV1.o GaugeV1.o StatsV1.o
EXEC_GraphicV2T = Graphicv2T.exe

# Default targetffunc
all: $(EXEC_GraphicV2) $(EXEC_GraphicV2T)

# Link the executable for Graphicv2 with full static linking
$(EXEC_GraphicV2): $(OBJ_GraphicV2)
	$(CXX) -o $(EXEC_GraphicV2) $(OBJ_GraphicV2) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Graphicv2T with full static linking
$(EXEC_GraphicV2T): $(OBJ_GraphicV2T)
	$(CXX) -o $(EXEC_GraphicV2T) $(OBJ_GraphicV2T) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Compile C++ source files to object files
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Clean up build artifacts
clean:
	@echo Cleaning up build artifacts...
	-@del /q *.o *.exe 2> NUL || rm -f *.o *.exe

.PHONY: all clean