# Changelog

All notable changes to the F1 Game project will be documented in this file.

## [0.2.0] - 2025-06-06

### Major Changes
- **Migrated from Raylib to SFML + OpenGL** for true 3D rendering
- Replaced 2D scanline pseudo-3D with actual 3D polygon rendering
- Implemented OpenGL depth buffer for proper 3D depth testing
- Added OpenGL projection and view matrices for camera system
- Converted all rendering from Raylib to OpenGL immediate mode
- Replaced Raylib input handling with SFML keyboard system
- Updated CMake configuration to link SFML libraries instead of Raylib

### Technical Details
- Technology stack: C++17 + SFML 2.5 + OpenGL 2.1
- Build system: CMake with MinGW Makefiles
- Rendering: Hardware-accelerated 3D graphics with depth testing
- Input: SFML keyboard event system
- Camera: gluLookAt for 3D camera positioning

### Configuration
- Updated README with SFML installation instructions via pacman
- Removed Raylib-specific setup instructions
- Updated CMakeLists.txt to find_package(SFML) and link sfml-graphics, sfml-window, sfml-system
- Added OpenGL linking (opengl32 on Windows, GL on Linux)

### Bug Fixes
- Fixed Car.cpp rendering to use public Raylib API (DrawRectanglePro) instead of internal rl* functions
- Replaced internal matrix functions with proper 2D rotation calculations
- Fixed Track.cpp compilation errors by adding missing <cmath> include for math functions (cosf, sinf, sqrtf)
- Fixed linker error by correcting library name in CMakeLists.txt (raylib instead of libraylib.a)
- Fixed invisible car rendering by simplifying car draw logic and adding direction indicator
- Fixed broken track rendering by replacing complex segment math with proper ellipse approximation
- Fixed text smear effect in HUD by removing redundant ClearBackground and updating text colors for visibility
- Fixed curb/kerb rendering "saw blade" effect by removing radial curb pattern and using simple white borders
- Fixed road "funnel effect" at bottom by clipping rendering before cockpit area
- Added F1-style cockpit with Halo system, steering wheel, and rear car view
- Updated rendering order: ClearBackground -> Track -> Cockpit -> HUD for proper layering
- Fixed perspective scaling equation to use logarithmic formula for natural road narrowing
- Fixed road width overflow at horizon by adding clamping to prevent screen-wide asphalt strips
- Fixed unrealistic speed values by reducing maxSpeed from 300m/s to 100m/s (~360 km/h)
- Fixed narrowing conversion warnings in Car.cpp by adding explicit float casts

## [0.1.0] - 2025-06-06

### Added
- Initial project structure with CMake build system
- C++17 backend with game loop and state management
- F1 car physics system with acceleration, braking, turning, and handbrake
- Pseudo-3D perspective rendering using horizontal scanlines (OutRun-style)
- Curve-based track system with straights and turns
- Segment-based road rendering with perspective scaling
- Red/white alternating curbs for F1-style appearance
- Sky and grass background with horizon line
- Static car rendering at bottom center with visual turning effects
- Progress-based movement system instead of x,y coordinates
- HUD with speed display (km/h), FPS counter, and controls info
- Physics utilities for distance calculation, normalization, and rotation
- README.md with setup and build instructions
- CHANGELOG.md for tracking changes
- WASD keyboard controls in addition to arrow keys for better accessibility

### Technical Details
- Technology stack: C++17 + Raylib graphics library
- Build system: CMake with MinGW Makefiles
- Rendering: Hardware-accelerated 2D graphics at 60 FPS
- Input: Keyboard controls (Arrow keys, Space, R)

### Configuration
- Updated README to specify MSYS2 MINGW64 as the required environment
- Added warnings about using the correct MSYS2 terminal (MINGW64, not UCRT64, MSYS, CLANG64, or CLANGARM64)
- Added build tools installation instructions (cmake, make, gcc) via pacman
- Fixed Raylib installation instructions based on actual pacman package structure
- Updated file paths to match pacman installation: raylib.h, libraylib.a, libraylib.dll.a
- Configured project for static linking since pacman provides only static libraries
- Removed DLL copy step from CMakeLists.txt since raylib.dll is not provided by pacman
- Added MSYS2 path format instructions for navigating to project directory

### Bug Fixes
- Fixed Car.cpp rendering to use public Raylib API (DrawRectanglePro) instead of internal rl* functions
- Replaced internal matrix functions with proper 2D rotation calculations
- Fixed Track.cpp compilation errors by adding missing <cmath> include for math functions (cosf, sinf, sqrtf)
- Fixed linker error by correcting library name in CMakeLists.txt (raylib instead of libraylib.a)
- Fixed invisible car rendering by simplifying car draw logic and adding direction indicator
- Fixed broken track rendering by replacing complex segment math with proper ellipse approximation
- Fixed text smear effect in HUD by removing redundant ClearBackground and updating text colors for visibility
- Fixed curb/kerb rendering "saw blade" effect by removing radial curb pattern and using simple white borders
- Fixed road "funnel effect" at bottom by clipping rendering before cockpit area
- Added F1-style cockpit with Halo system, steering wheel, and rear car view
- Updated rendering order: ClearBackground -> Track -> Cockpit -> HUD for proper layering
- Fixed perspective scaling equation to use logarithmic formula for natural road narrowing
- Fixed road width overflow at horizon by adding clamping to prevent screen-wide asphalt strips
- Fixed unrealistic speed values by reducing maxSpeed from 300m/s to 100m/s (~360 km/h)
- Fixed narrowing conversion warnings in Car.cpp by adding explicit float casts
