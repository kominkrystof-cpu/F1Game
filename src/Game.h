#ifndef GAME_H
#define GAME_H

#include "Car.h"
#include "Track.h"
#include "Graphics.h"
#include "Shader.h"
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Game {
public:
    Game();
    ~Game();

    bool Init();
    void Run();
    void Cleanup();

private:
    void Update();
    void Render();
    void HandleInput();
    void SetupOpenGL();
    void DrawHUD();
    void DrawCockpit();
    bool LoadFont();

    int screenWidth;
    int screenHeight;
    const char* title;

    sf::RenderWindow window;
    Car playerCar;
    Track track;
    bool running;
    float deltaTime;

    sf::Font font;
    bool fontLoaded;
    sf::Text* speedText;
    sf::Text* lapText;
    sf::Text* fpsText;
    sf::Text* controlsText;
    sf::Text* gearText;
    float fpsTimer;
    int frameCount;

    int currentLap;
    int totalLaps;
    float lastProgress;
    unsigned int carbonTex;

    // Modern OpenGL resources
    GLuint trackShaderProgram;
    GLuint carShaderProgram;
    GLuint trackVAO;
    GLuint trackVBO;
    GLuint trackEBO;
    GLuint trackIndexCount;

    void CreateTrackGeometry();
};

#endif
