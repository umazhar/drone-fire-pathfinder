#ifndef PERCEPTION_MAP_HPP
#define PERCEPTION_MAP_HPP

#include <vector>
#include "GridMap.hpp"

class PerceptionMap {
public:
    PerceptionMap(int width, int height, int detectionRange);

    // Update what the drone perceives (and remembers) 
    // based on current position and the actual GridMap.
    void updatePerception(const GridMap& map, int droneX, int droneY);

    // For debugging or communication: get raw data about which cells are known as fire.
    bool isFireKnownAt(int x, int y) const;

    // Optional: returns the entire 2D array if you need to send it to CommModule
    const std::vector<std::vector<bool>>& getKnownFires() const;

private:
    int m_width;
    int m_height;
    int m_detectionRange;

    // 2D array storing whether we have *ever* seen fire at [y][x].
    // Could also store current and historical separately if you want.
    std::vector<std::vector<bool>> m_fireKnown;
};

#endif // PERCEPTION_MAP_HPP
