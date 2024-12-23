# 2D Pathfinding and Fire Monitoring Algorithm for a Drone

This project simulates a drone navigating a 2D grid-based environment to monitor and locate fires, simulating real-world UAV capabilities. It features dynamic fire spread, perception updates, and basic data communication between the drone and a ground station. 

## How to Compile

### Using Code Runner in Visual Studio Code
1. Install the **Code Runner** extension.
2. Update the executor map to support modern C++ standards:
   - Open the extension settings in VS Code.
   - Add the following argument to `"cpp"` in the `code-runner.executorMap` setting:
     ```cpp
     "-std=c++17"
     ``` 
     so it should look something like this:
    ```json
    "cpp": "cd $dir && g++ -std=c++17 $fileName -o $fileNameWithoutExt && $dir$fileNameWithoutExt",
    ```
3. Save your settings and run the project.

### Alternative Compilation (Command Line)
If you're not using Code Runner:
1. Open a terminal in the project root directory.
2. Run the makefile
   ```bash
   make run`
   ```
