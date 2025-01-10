CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./include

SRCS = src/GridMap.cpp src/FireSpreadSimulator.cpp src/Drone.cpp \
       src/PerceptionMap.cpp src/Pathfinder.cpp src/CommModule.cpp src/main.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = drone_fire_sim

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c $< -o $@

clean:
    rm -f $(OBJS) $(TARGET)
