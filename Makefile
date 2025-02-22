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

# Project files for Functions_testV1.cpp
SRC_FN1 = StatsV1.cpp Functions_testV1.cpp
OBJ_FN1 = StatsV1.o Functions_testV1.o
EXEC_FN1 = Functions_testV1.exe

# Project files for Graphicv0
SRC_GraphicV0 = Graphicv0.cpp Bar.cpp Gauge.cpp Stats.cpp StatsV1.cpp
OBJ_GraphicV0 = Graphicv0.o Bar.o Gauge.o Stats.o StatsV1.o
EXEC_GraphicV0 = Graphicv0.exe

# Project files for Graphicv1
SRC_GraphicV1 = Graphicv1.cpp BarV1.cpp GaugeV1.cpp Stats.cpp StatsV1.cpp
OBJ_GraphicV1 = Graphicv1.o BarV1.o GaugeV1.o Stats.o StatsV1.o
EXEC_GraphicV1 = Graphicv1.exe

# Project files for Graphicv2
SRC_GraphicV2 = Graphicv2.cpp BarV1.cpp GaugeV1.cpp StatsV1.cpp
OBJ_GraphicV2 = Graphicv2.o BarV1.o GaugeV1.o StatsV1.o
EXEC_GraphicV2 = Graphicv2.exe

# Default targetffunc
all: $(EXEC_GraphicV2)

# Link the executable for Functions_test with full static linking
$(EXEC_FN): $(OBJ_FN)
	$(CXX) -o $(EXEC_FN) $(OBJ_FN) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Functions_testV1 with full static linking
$(EXEC_FN1): $(OBJ_FN1)
	$(CXX) -o $(EXEC_FN1) $(OBJ_FN1) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Graphicv0 with full static linking
$(EXEC_GraphicV0): $(OBJ_GraphicV0)
	$(CXX) -o $(EXEC_GraphicV0) $(OBJ_GraphicV0) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Graphicv1 with full static linking
$(EXEC_GraphicV1): $(OBJ_GraphicV1)
	$(CXX) -o $(EXEC_GraphicV1) $(OBJ_GraphicV1) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Link the executable for Graphicv2 with full static linking
$(EXEC_GraphicV2): $(OBJ_GraphicV2)
	$(CXX) -o $(EXEC_GraphicV2) $(OBJ_GraphicV2) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Compile C++ source files to object files
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Clean up build artifacts
clean:
	@echo Cleaning up build artifacts...
	-@del /q *.o *.exe 2> NUL || rm -f *.o *.exe

.PHONY: all clean