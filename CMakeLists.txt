cmake_minimum_required(VERSION 3.15)
project(F1Game VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SFML setup
find_package(SFML REQUIRED COMPONENTS Window Graphics System)
# GLEW setup
find_package(GLEW REQUIRED)
# GLM setup (header-only)
find_package(GLM REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Car.cpp
    src/Track.cpp
    src/Physics.cpp
    src/Graphics.cpp
    src/Shader.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link SFML and OpenGL
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        sfml-graphics
        sfml-window
        sfml-system
        opengl32
        glu32
        ${GLEW_LIBRARIES}
    )
else()
    target_link_libraries(${PROJECT_NAME}
        sfml-graphics
        sfml-window
        sfml-system
        GL
        GLU
        ${GLEW_LIBRARIES}
    )
endif()

# Add GLM include directories (header-only library)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIRS})
