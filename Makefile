# Compiler
CXX = g++

# Compiler Flags
CXXFLAGS = -Wall -Wextra -std=c++11

# Linker Flags
LDFLAGS = -lpdh

# Executable Name
EXEC = system_stats

# Source Files
SRCS = main.cpp stats.cpp

# Object Files
OBJS = $(SRCS:.cpp=.o)

# Default Target
all: $(EXEC)

# Link the object files to create the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS) $(LDFLAGS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean the project directory by removing object files and the executable
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets to avoid conflicts with files named 'clean' or 'all'
.PHONY: all clean
