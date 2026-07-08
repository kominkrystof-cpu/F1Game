# F1 Game

F1 racing game built with C++ and SFML + OpenGL graphics library.

## Requirements
- MSYS2 MinGW64
- C++17 compatible compiler

## Setup

### Install Build Tools
**Important: Use "MSYS2 MINGW64" terminal** (not UCRT64, MSYS, CLANG64, or CLANGARM64)

Open MSYS2 MINGW64 terminal and run:
```bash
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-gcc
```

### Install SFML
```bash
pacman -S mingw-w64-x86_64-sfml
```

## Build

**Important: Use "MSYS2 MINGW64" terminal** (not UCRT64, MSYS, CLANG64, or CLANGARM64)

Navigate to project directory using MSYS2 path format:
```bash
cd /c/Users/komin/Absolute\ dogshit\ coding\ projects/F1-Game
```

Then build:
```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

## Run

```bash
./F1Game.exe
```

## Controls
- Arrow Up / W: Accelerate
- Arrow Down / S: Brake/Reverse
- Arrow Left / A: Turn Left
- Arrow Right / D: Turn Right
- Space: Handbrake
- R: Reset car position

## Features
- Realistic F1 car physics
- Oval track with grass and asphalt
- Speed display in km/h
- Collision detection with track boundaries
- Smooth 60 FPS rendering
