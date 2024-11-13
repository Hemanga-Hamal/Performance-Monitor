# Compiler
CXX = g++

# Compiler and flags
CXX = g++

# Full static linking
STATIC_FLAGS = -static -static-libgcc -static-libstdc++ -lpdh

# Source files
SRC_Main = main.cpp
OBJ_Main = main.o
EXEC_Main = main.exe

# Build target
all: $(EXEC_Main)

$(EXEC_Main): $(OBJ_Main)
	$(CXX) $(CXXFLAGS) $(STATIC_FLAGS) $^ -o $@ 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	@echo Cleaning up build artifacts...
	-@del /q *.o *.exe 2> NUL || rm -f *.o *.exe

.PHONY: all clean