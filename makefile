# Define the source directory
SRC_DIR = src

# Define the output executables
BASE_STATION_SERVER = BaseStationServer
DRONE_CLIENT = DroneClient

# Define the compiler and flags
CXX = g++
CXXFLAGS = -lws2_32

# Build all targets
all: $(BASE_STATION_SERVER) $(DRONE_CLIENT)

# Compile BaseStationServer
$(BASE_STATION_SERVER): $(SRC_DIR)/BaseStationServer.cpp
	$(CXX) $< -o $@ $(CXXFLAGS)

# Compile DroneClient
$(DRONE_CLIENT): $(SRC_DIR)/DroneClient.cpp $(SRC_DIR)/GridMap.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS)

# Clean build artifacts
clean:
	rm -f $(BASE_STATION_SERVER) $(DRONE_CLIENT)
