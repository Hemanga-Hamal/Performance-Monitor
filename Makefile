# Paths
RAYLIB_PATH = C:/raylib/raylib
SRCDIR = src

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I$(SRCDIR) -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external -DPLATFORM_DESKTOP
LDFLAGS = -L$(RAYLIB_PATH)/src -L$(RAYLIB_PATH)/src/external

# Full static linking
STATIC_FLAGS = -static -static-libgcc -static-libstdc++ -lpdh -lws2_32 -lpsapi -mwindows

# Raylib linking (assuming static library is available)
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm $(STATIC_FLAGS)

# Source files
SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/BarV1.cpp $(SRCDIR)/GaugeV1.cpp $(SRCDIR)/StatsV1.cpp
OBJS = main.o BarV1.o GaugeV1.o StatsV1.o
EXEC = perfmon.exe

# Default target
all: $(EXEC)

# Link the executable
$(EXEC): $(OBJS)
	$(CXX) -o $(EXEC) $(OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

# Compile source files from src/
main.o: $(SRCDIR)/main.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

BarV1.o: $(SRCDIR)/BarV1.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

GaugeV1.o: $(SRCDIR)/GaugeV1.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

StatsV1.o: $(SRCDIR)/StatsV1.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Clean up build artifacts
clean:
	@echo Cleaning up build artifacts...
	-@del /q *.o *.exe 2> NUL || rm -f *.o *.exe

.PHONY: all clean
