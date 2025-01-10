#include "../include/Drone.hpp"

Drone::Drone(int startX, int startY, DroneMode mode)
    : m_x(startX), m_y(startY), m_mode(mode)
{
}

void Drone::moveTo(int newX, int newY) {
    // For now, just set the new coordinates.
    // In practice, you’d validate that the move is valid, 
    // check pathfinding, etc.
    m_x = newX;
    m_y = newY;
}

void Drone::setMode(DroneMode mode) {
    m_mode = mode;
}

std::pair<int,int> Drone::getPosition() const {
    return {m_x, m_y};
}
