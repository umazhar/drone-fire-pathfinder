CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

SRC_DIR = src
INC_DIR = include

TARGET  = drone_sim

# List of source files 
SOURCES = \
    $(SRC_DIR)/main.cpp \
    $(SRC_DIR)/GridMap.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

# Default rule: build the executable
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) $^ -o $@

# Rule to compile each .cpp into an .o
# $< is the first prerequisite (the .cpp)
# $@ is the target (the .o)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c $< -o $@

run: $(TARGET)
	@echo "Running the $(TARGET) executable..."
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean run
