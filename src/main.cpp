#include <iostream>
#include "../include/GridMap.hpp"
#include "../include/FireSpreadSimulator.hpp"
#include "../include/Drone.hpp"
#include "../include/PerceptionMap.hpp"
#include "../include/Pathfinder.hpp"
#include "../include/CommModule.hpp"

int main() {
    // 1. Initialize the map
    GridMap gridMap(50, 50, WindDirection::NORTH);

    // 2. Create FireSpreadSimulator
    FireSpreadSimulator simulator(WindDirection::NORTH);

    // 3. Create a Drone and PerceptionMap
    Drone drone(0, 0, DroneMode::SCAN);
    PerceptionMap perceptionMap(50, 50, 5); // detectionRange = 5

    // 4. (Optional) Some initial pathfinding usage
    // e.g., find a path to (10,10)
    // auto path = Pathfinder::findPath(gridMap, 0, 0, 10, 10);

    // 5. Main simulation loop (example: 10 timesteps)
    for (int t = 0; t < 10; ++t) {
        // Step A: Fire spread
        simulator.spreadFires(gridMap);

        // Step B: Drone moves (for demonstration, let’s just move +1 x each loop)
        auto currentPos = drone.getPosition();
        int newX = currentPos.first + 1;
        int newY = currentPos.second;
        drone.moveTo(newX, newY);

        // Step C: Update perception
        perceptionMap.updatePerception(gridMap, newX, newY);

        // Step D: Serialize and send data to base station
        auto data = CommModule::serializePerception(perceptionMap);
        CommModule::sendToBaseStation(data, "127.0.0.1", 9000);

        // Step E: Print or debug something
        std::cout << "Timestep " << t << " completed.\n";
    }

    return 0;
}
