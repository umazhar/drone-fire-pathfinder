#ifndef DRONE_HPP
#define DRONE_HPP

#include <utility>   // for std::pair
#include "GridMap.hpp"  // might need to see map bounds
// Include "PerceptionMap.hpp" only if needed within Drone

enum class DroneMode {
    MONITOR,
    SCAN
};

class Drone {
public:
    Drone(int startX, int startY, DroneMode mode);

    // Move the drone to a new position (should be done in small steps).
    void moveTo(int newX, int newY);

    // Switch between monitor and scan.
    void setMode(DroneMode mode);

    // Get current position.
    std::pair<int,int> getPosition() const;

private:
    int m_x;
    int m_y;
    DroneMode m_mode;

    // If you want to store a target fire location or something similar, 
    // add that here.
};

#endif // DRONE_HPP
