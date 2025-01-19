# 2D Pathfinding and Fire Monitoring Algorithm for a Drone

This project simulates a drone navigating a 2D grid-based environment to monitor and locate fires, simulating real-world UAV capabilities. It features dynamic fire spread, perception updates, and basic data communication between the drone and a ground station. The system is split into two programs:
- **Base Station (Server):** Receives fire alerts from the drone and updates a live map as fires are discovered.
- **Drone (Client):** Simulates scanning a grid, discovering fires, and reporting them to the Base Station.

## How to Compile

#### For Base Station (Server)
1. Open a terminal in the project's directory.
2. Compile the server code:
   ```bash
   g++ BaseStationServer.cpp -o BaseStationServer -lws2_32
   ```
3. Run the server:
   ```bash
   ./BaseStationServer
   ```
   On Windows, run:
   ```bash
   BaseStationServer.exe
   ```

#### For Drone (Client)
1. In the terminal, compile the client code along with `GridMap.cpp`:
   ```bash
   g++ DroneClient.cpp GridMap.cpp -o DroneClient -lws2_32
   ```
2. Run the drone client:
   ```bash
   ./DroneClient
   ```
   On Windows, run:
   ```bash
   DroneClient.exe
   ```

## How to Run

1. **Start the Base Station Server:**
   - Open a terminal and run `BaseStationServer`. The server will listen on port 12345 for a drone connection.
   - The server displays a blank 15×15 grid. It will update the map live as it receives fire reports from the drone.

2. **Launch the Drone Client:**
   - In a separate terminal, run `DroneClient`. The drone will connect to the base station, receive the initial grid size and map, and start scanning for fires.
   - As the drone discovers fires, it sends updates (`FIRE r c`) to the server. Meanwhile, it displays its own live map with the drone’s position and discovered fires.

3. **Live Interaction:**
   - On the **drone side**, you will see a live-updating map as it scans the grid.
   - On the **server side**, each time the drone reports a new fire, the server map will update live to show the newly discovered fire cells.

4. **Completion:**
   - Once scanning is complete or the drone cannot continue, it sends an `END` message to the server.
   - The server then prints the final discovered fire map and stops receiving further updates.