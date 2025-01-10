#include "../include/PerceptionMap.hpp"

PerceptionMap::PerceptionMap(int width, int height, int detectionRange)
    : m_width(width), m_height(height), m_detectionRange(detectionRange)
{
    m_fireKnown.resize(m_height, std::vector<bool>(m_width, false));
}

void PerceptionMap::updatePerception(const GridMap& map, int droneX, int droneY) {
    // For all cells within m_detectionRange, check if they’re on fire in the actual map.
    // If yes, mark m_fireKnown[y][x] = true.
    // Pseudocode:
    /*
    for (int dy = -m_detectionRange; dy <= m_detectionRange; dy++) {
        for (int dx = -m_detectionRange; dx <= m_detectionRange; dx++) {
            int checkX = droneX + dx;
            int checkY = droneY + dy;
            if (checkX >= 0 && checkX < m_width && 
                checkY >= 0 && checkY < m_height) {
                
                int cellState = map.getCellState(checkX, checkY);
                if (cellState == 1) { // assuming 1 means fire
                    m_fireKnown[checkY][checkX] = true;
                }
            }
        }
    }
    */
}

bool PerceptionMap::isFireKnownAt(int x, int y) const {
    if (y < 0 || y >= m_height || x < 0 || x >= m_width) {
        return false;
    }
    return m_fireKnown[y][x];
}

const std::vector<std::vector<bool>>& PerceptionMap::getKnownFires() const {
    return m_fireKnown;
}
