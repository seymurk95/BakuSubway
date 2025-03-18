# Baku Subway Simulation

This program simulates the operation of the Baku Subway system using multithreading to manage train movement on different lines. It demonstrates the use of mutexes for thread synchronization to prevent race conditions when multiple trains access shared resources (stations).

## Files Overview

### 1) **BakuSubway.h**
A header file defining the `BakuSubway` class and its core functionality:
- **Line Structure**: Defines subway lines, stations, and depots.
- **Train Movement Logic**: Handles train movement in both directions.
- **Thread Synchronization**: Uses mutexes to ensure safe access to shared stations.
- **Safe Output Printing**: Uses a mutex to prevent console output corruption in multithreaded execution.

### 2) **BakuSubway.cpp**
Contains the implementation of the subway simulation:
- **Constructor**: Initializes subway lines and station mutexes.
- **Train Function**: Simulates train movement along the line, ensuring exclusive access to stations.
- **Safe Print Function**: Prevents concurrent console output from multiple threads.
- **Run Function**: Starts the subway simulation by spawning train threads.

### 3) **main.cpp**
The entry point of the program:
- Creates an instance of `BakuSubway`.
- Calls the `run()` method to start the simulation.

### 4) **CMakeLists.txt**
A CMake configuration file for building the project:
- Specifies C++20 as the standard.
- Defines `BakuSubway` as the executable target.

## How It Works
1. The subway system initializes lines and stations.
2. Multiple trains are simulated using threads.
3. Trains move along their assigned routes, locking and unlocking station mutexes to prevent collisions.
4. The simulation prints train movements safely using synchronized output.


## Example Output
```
Train 1 is at Station A
Train 2 is at Station B
Train 1 is moving to Station B
Train 2 is moving to Station C
...
```

## Dependencies
- C++20 or later
- CMake (for easy compilation)

This project demonstrates basic multithreading concepts and synchronization using mutexes while simulating a real-world subway system.

