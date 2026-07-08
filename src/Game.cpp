#include "Game.h"
#include "Graphics.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <algorithm>

Game::Game() : screenWidth(1280), screenHeight(720), title("F1 Racing Game"),
               running(true), deltaTime(0.0f), fpsTimer(0.0f), frameCount(0),
               fontLoaded(false), currentLap(1), totalLaps(15),
               lastProgress(0.0f), carbonTex(0),
               trackShaderProgram(0), carShaderProgram(0), trackVAO(0), trackVBO(0), trackEBO(0), trackIndexCount(0) {
}

Game::~Game() {
    delete speedText;
    delete lapText;
    delete fpsText;
    delete controlsText;
    delete gearText;
}

bool Game::LoadFont() {
    const char* paths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/consola.ttf"
    };
    for (const char* path : paths) {
        if (font.openFromFile(path)) {
            return true;
        }
    }
    return false;
}

bool Game::Init() {
    // Request OpenGL 3.3 Core Profile context
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antiAliasingLevel = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.attributeFlags = sf::ContextSettings::Core;

    window.create(sf::VideoMode({static_cast<unsigned int>(screenWidth), static_cast<unsigned int>(screenHeight)}),
                  title, sf::Style::Close, sf::State::Windowed, settings);
    window.setFramerateLimit(60);

    (void)window.setActive(true);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    SetupOpenGL();

    // Load shaders
    trackShaderProgram = Graphics::CreateShaderProgram(
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;     // Position\n"
        "layout (location = 1) in vec3 aNormal;  // Normal\n"
        "layout (location = 2) in vec2 aTexCoord; // TexCoord\n"
        "\n"
        "out vec3 FragPos;\n"
        "out vec3 Normal;\n"
        "out vec2 TexCoord;\n"
        "\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
        "    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "    TexCoord = aTexCoord;\n"
        "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
        "}",
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "\n"
        "// Inputs from vertex shader\n"
        "in vec3 FragPos;\n"
        "in vec3 Normal;\n"
        "in vec2 TexCoord;\n"
        "\n"
        "// Texture sampler\n"
        "uniform sampler2D textureSampler;\n"
        "uniform bool useTexture;\n"
        "\n"
        "// Material properties\n"
        "uniform vec3 ambient;\n"
        "uniform vec3 diffuse;\n"
        "uniform vec3 specular;\n"
        "uniform float shininess;\n"
        "\n"
        "// Light properties\n"
        "uniform vec3 lightPos;\n"
        "uniform vec3 viewPos;\n"
        "uniform vec3 lightColor;\n"
        "\n"
        "// Fog settings (optional)\n"
        "uniform float fogDensity;\n"
        "uniform vec3 fogColor;\n"
        "uniform bool useFog;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    // Ambient\n"
        "    vec3 ambientResult = ambient * lightColor;\n"
        "\n"
        "    // Diffuse\n"
        "    vec3 norm = normalize(Normal);\n"
        "    vec3 lightDir = normalize(lightPos - FragPos);\n"
        "    float diff = max(dot(norm, lightDir), 0.0);\n"
        "    vec3 diffuseResult = diff * diffuse * lightColor;\n"
        "\n"
        "    // Specular\n"
        "    vec3 viewDir = normalize(viewPos - FragPos);\n"
        "    vec3 reflectDir = reflect(-lightDir, norm);\n"
        "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
        "    vec3 specularResult = spec * specular * lightColor;\n"
        "\n"
        "    // Combine results\n"
        "    vec3 result = (ambientResult + diffuseResult + specularResult);\n"
        "\n"
        "    // Sample texture if enabled\n"
        "    vec4 texColor = texture(textureSampler, TexCoord);\n"
        "    if (useTexture) {\n"
        "        result = result * texColor.rgb;\n"
        "    }\n"
        "\n"
        "    // Fog (optional)\n"
        "    if (useFog) {\n"
        "        float distance = length(FragPos - viewPos);\n"
        "        float fogFactor = exp(-fogDensity * distance * distance);\n"
        "        fogFactor = clamp(fogFactor, 0.0, 1.0);\n"
        "        result = mix(result, fogColor, fogFactor);\n"
        "    }\n"
        "\n"
        "    FragColor = vec4(result, texColor.a);\n"
        "}"
    );

    if (trackShaderProgram == 0) {
        std::cerr << "Failed to create track shader program" << std::endl;
        return false;
    }

    // For now, car uses the same shader as track (we can differentiate later)
    carShaderProgram = trackShaderProgram;

    fontLoaded = LoadFont();
    if (!fontLoaded) {
        std::cerr << "Warning: Could not load any system font for HUD" << std::endl;
    }

    speedText = new sf::Text(font, "", 26);
    speedText->setFillColor(sf::Color::White);
    speedText->setStyle(sf::Text::Bold);
    speedText->setPosition({28.0f, 20.0f});

    lapText = new sf::Text(font, "", 24);
    lapText->setFillColor(sf::Color(230, 230, 230));
    lapText->setStyle(sf::Text::Bold);
    lapText->setPosition({static_cast<float>(screenWidth - 185), 20.0f});

    fpsText = new sf::Text(font, "", 13);
    fpsText->setFillColor(sf::Color(120, 210, 120));
    fpsText->setPosition({static_cast<float>(screenWidth - 75), static_cast<float>(screenHeight - 22)});

    controlsText = new sf::Text(font, "", 14);
    controlsText->setFillColor(sf::Color(190, 190, 190));
    controlsText->setPosition({24.0f, static_cast<float>(screenHeight - 48)});

    gearText = new sf::Text(font, "N", 20);
    gearText->setFillColor(sf::Color(0, 230, 80));
    gearText->setStyle(sf::Text::Bold);
    gearText->setPosition({screenWidth / 2.0f - 8.0f, static_cast<float>(screenHeight) - 82.0f});

    track.CreateDefaultTrack();
    track.InitGraphics();
    carbonTex = Graphics::CreateCarbonTexture();
    playerCar.Init();
    lastProgress = playerCar.GetProgress();

    return true;
}

void Game::SetupOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Graphics::SetupPerspective(screenWidth, screenHeight);

    // Setup camera (we'll update this each frame in Render)
    glm::vec3 cameraPos(0.0f, 1.25f, 0.0f);
    glm::vec3 cameraTarget(0.0f, 1.0f, -100.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    Graphics::SetupCamera(cameraPos, cameraTarget, cameraUp);

    glClearColor(0.68f, 0.82f, 0.94f, 1.0f);

    // Setup lighting for our shader
    Graphics::SetupLighting(trackShaderProgram);
    Graphics::SetupFog(trackShaderProgram);
}

void Game::Run() {
    sf::Clock clock;
    while (running && window.isOpen()) {
        deltaTime = clock.restart().asSeconds();
        HandleInput();
        Update();
        Render();
    }
}

void Game::Cleanup() {
    // Cleanup OpenGL resources
    if (trackVAO != 0) {
        Graphics::DeleteVAO(trackVAO);
    }
    if (trackVBO != 0) {
        Graphics::DeleteVBO(trackVBO);
    }
    if (trackEBO != 0) {
        Graphics::DeleteEBO(trackEBO);
    }

    if (trackShaderProgram != 0) {
        Graphics::DeleteShaderProgram(trackShaderProgram);
    }

    Graphics::DeleteTexture(carbonTex);
    track.Cleanup();
    window.close();
}

void Game::CreateTrackGeometry() {
    // Create a simple track as a series of quads for demonstration
    // In a real implementation, this would use the actual track data from Track.cpp

    // For now, let's create a simple flat track segment to test our rendering
    float vertices[] = {
        // Positions          // Normals           // TexCoords
        -50.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         50.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         50.0f, 0.0f,  50.0f, 0.0f, 1.0f, 0.0f,  1.0f, 1.0f,

        -50.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         50.0f, 0.0f,  50.0f, 0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -50.0f, 0.0f,  50.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        3, 4, 5   // Second triangle
    };

    trackIndexCount = 6; // 2 triangles * 3 vertices

    // Generate and bind VAO
    trackVAO = Graphics::CreateVAO();
    glBindVertexArray(trackVAO);

    // Generate and bind VBO
    trackVBO = Graphics::CreateVBO(sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, trackVBO);

    // Generate and bind EBO
    trackEBO = Graphics::CreateEBO(sizeof(indices), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, trackEBO);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbind VAO
    glBindVertexArray(0);
}

void Game::HandleInput() {
    while (std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            running = false;
        }
    }

    float steer = 0.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        steer -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        steer += 1.0f;
    }
    playerCar.SetSteeringInput(steer);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        playerCar.Accelerate(deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        playerCar.Brake(deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        playerCar.Handbrake(deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
        playerCar.Reset();
        currentLap = 1;
        lastProgress = 0.0f;
    }
}

void Game::Update() {
    float prevProgress = playerCar.GetProgress();
    playerCar.Update(deltaTime, track);
    float newProgress = playerCar.GetProgress();
    
    if (newProgress < lastProgress && prevProgress > track.GetTrackLength() * 0.9f) {
        currentLap++;
        if (currentLap > totalLaps) currentLap = 1;
    }
    lastProgress = newProgress;
}

void Game::DrawCockpit() {
    float cockpitY = static_cast<float>(screenHeight) - 220.0f;
    float wheelTurn = playerCar.GetRotation() * 90.0f;
    
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    
    // Cockpit tub - shaded quads (no texture to avoid SFML conflict)
    glBegin(GL_QUADS);
    glColor3f(0.12f, 0.12f, 0.13f);
    glVertex2f(0.0f, cockpitY);
    glVertex2f(static_cast<float>(screenWidth), cockpitY);
    glColor3f(0.08f, 0.08f, 0.09f);
    glVertex2f(static_cast<float>(screenWidth), static_cast<float>(screenHeight));
    glVertex2f(0.0f, static_cast<float>(screenHeight));
    glEnd();
    
    // Side pillars
    auto drawPillar = [&](float cx, bool left) {
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.07f, 0.07f, 0.08f);
        glVertex2f(cx, cockpitY - 60.0f);
        for (int i = 0; i <= 24; i++) {
            float angle = 3.14159f * i / 24.0f;
            float px = left ? cx + 90.0f * cosf(angle) : cx - 90.0f * cosf(angle);
            float py = cockpitY - 60.0f + 90.0f * sinf(angle);
            float shade = 0.10f + 0.07f * sinf(angle);
            glColor3f(shade, shade, shade + 0.01f);
            glVertex2f(px, py);
        }
        glEnd();
    };
    drawPillar(0.0f, true);
    drawPillar(static_cast<float>(screenWidth), false);
    
    // Halo
    glColor3f(0.04f, 0.04f, 0.05f);
    glBegin(GL_QUADS);
    glVertex2f(screenWidth / 2.0f - 14.0f, cockpitY - 110.0f);
    glVertex2f(screenWidth / 2.0f + 14.0f, cockpitY - 110.0f);
    glVertex2f(screenWidth / 2.0f + 14.0f, cockpitY);
    glVertex2f(screenWidth / 2.0f - 14.0f, cockpitY);
    glEnd();
    
    glColor3f(0.03f, 0.03f, 0.04f);
    glBegin(GL_QUADS);
    glVertex2f(screenWidth / 2.0f - 70.0f, cockpitY - 85.0f);
    glVertex2f(screenWidth / 2.0f - 14.0f, cockpitY - 85.0f);
    glVertex2f(screenWidth / 2.0f - 14.0f, cockpitY - 25.0f);
    glVertex2f(screenWidth / 2.0f - 70.0f, cockpitY - 25.0f);
    glVertex2f(screenWidth / 2.0f + 14.0f, cockpitY - 85.0f);
    glVertex2f(screenWidth / 2.0f + 70.0f, cockpitY - 85.0f);
    glVertex2f(screenWidth / 2.0f + 70.0f, cockpitY - 25.0f);
    glVertex2f(screenWidth / 2.0f + 14.0f, cockpitY - 25.0f);
    glEnd();
    
    // Steering wheel
    float wheelX = screenWidth / 2.0f;
    float wheelY = static_cast<float>(screenHeight) - 75.0f;
    float wheelRadius = 55.0f;
    
    glPushMatrix();
    glTranslatef(wheelX, wheelY, 0.0f);
    glRotatef(wheelTurn, 0.0f, 0.0f, 1.0f);
    
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 64; i++) {
        float angle = 3.14159f * i / 32.0f;
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        float shade = 0.16f + 0.07f * sinA;
        glColor3f(shade, shade, shade + 0.02f);
        glVertex2f(cosA * (wheelRadius - 10.0f), sinA * (wheelRadius - 10.0f));
        glColor3f(shade + 0.05f, shade + 0.05f, shade + 0.07f);
        glVertex2f(cosA * wheelRadius, sinA * wheelRadius);
    }
    glEnd();
    
    glColor3f(0.26f, 0.26f, 0.28f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 64; i++) {
        float angle = 3.14159f * i / 32.0f;
        glVertex2f(cosf(angle) * (wheelRadius - 14.0f), sinf(angle) * (wheelRadius - 14.0f));
        glVertex2f(cosf(angle) * (wheelRadius - 8.0f), sinf(angle) * (wheelRadius - 8.0f));
    }
    glEnd();
    
    glColor3f(0.13f, 0.13f, 0.15f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(-12.0f, -wheelRadius + 12.0f);
    glVertex2f(12.0f, -wheelRadius + 12.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(-wheelRadius + 12.0f, -8.0f);
    glVertex2f(-wheelRadius + 12.0f, 8.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(wheelRadius - 12.0f, -8.0f);
    glVertex2f(wheelRadius - 12.0f, 8.0f);
    glEnd();
    
    glColor3f(0.05f, 0.05f, 0.06f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= 32; i++) {
        float angle = 6.28318f * i / 32.0f;
        glVertex2f(cosf(angle) * 22.0f, sinf(angle) * 22.0f);
    }
    glEnd();
    
    glColor3f(0.02f, 0.02f, 0.03f);
    glBegin(GL_QUADS);
    glVertex2f(-22.0f, -12.0f);
    glVertex2f(22.0f, -12.0f);
    glVertex2f(22.0f, 12.0f);
    glVertex2f(-22.0f, 12.0f);
    glEnd();
    
    glPopMatrix();
    
    // Dashboard + rev lights
    glColor3f(0.09f, 0.09f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(screenWidth / 2.0f - 120.0f, cockpitY - 5.0f);
    glVertex2f(screenWidth / 2.0f + 120.0f, cockpitY - 5.0f);
    glVertex2f(screenWidth / 2.0f + 120.0f, cockpitY + 15.0f);
    glVertex2f(screenWidth / 2.0f - 120.0f, cockpitY + 15.0f);
    glEnd();
    
    float revRatio = playerCar.GetSpeed() / playerCar.GetMaxSpeed();
    for (int i = 0; i < 10; i++) {
        float lx = screenWidth / 2.0f - 90.0f + i * 20.0f;
        float threshold = static_cast<float>(i + 1) / 10.0f;
        if (revRatio >= threshold) {
            if (i < 7) glColor3f(0.0f, 0.75f, 0.2f);
            else if (i < 9) glColor3f(0.85f, 0.65f, 0.0f);
            else glColor3f(0.85f, 0.1f, 0.1f);
        } else {
            glColor3f(0.14f, 0.14f, 0.15f);
        }
        glBegin(GL_QUADS);
        glVertex2f(lx, cockpitY + 2.0f);
        glVertex2f(lx + 14.0f, cockpitY + 2.0f);
        glVertex2f(lx + 14.0f, cockpitY + 12.0f);
        glVertex2f(lx, cockpitY + 12.0f);
        glEnd();
    }
    
    glEnable(GL_DEPTH_TEST);
}

void Game::Render() {
    (void)window.setActive(true);

    // SFML resetGLStates() at end of previous frame destroys the 3D projection matrix.
    // Must restore perspective before every 3D draw pass.
    Graphics::SetupPerspective(screenWidth, screenHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float progress = playerCar.GetProgress();
    float speedFactor = playerCar.GetSpeed() * 0.008f;
    float cameraSway = sinf(progress * 0.15f) * speedFactor;
    float cameraBob = sinf(progress * 0.3f) * speedFactor * 0.3f;

    // Modern OpenGL camera setup using GLM
    glm::vec3 cameraPos(cameraSway, 1.25f + cameraBob, 0.0f);
    glm::vec3 cameraTarget(cameraSway, 1.0f, -100.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    Graphics::SetupCamera(cameraPos, cameraTarget, cameraUp);

    // Update lighting for the current shader program
    Graphics::SetupLighting(trackShaderProgram);

    // Begin modern track rendering
    Graphics::BeginTrackRender(trackShaderProgram);

    // Render track using modern OpenGL (delegated to Track class)
    track.Draw3D(progress, playerCar.GetSpeed(), trackShaderProgram);

    Graphics::EndTrackRender();

    // Legacy track rendering as fallback during transition
    // track.Render(progress, playerCar.GetSpeed());

    // Draw sky dome (legacy - to be updated later)
    float heading = track.GetTrackHeading(progress);
    Graphics::DrawSkyDome(heading, progress);

    // OpenGL cockpit overlay (keeping legacy for now)
    window.pushGLStates();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    DrawCockpit();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    window.popGLStates();

    // SFML HUD - must be drawn with clean GL state
    window.resetGLStates();
    DrawHUD();

    window.display();
}

void Game::DrawHUD() {
    if (!fontLoaded) return;
    
    float speed = playerCar.GetSpeed();
    int speedKmh = static_cast<int>(speed * 100.0f);
    int gear = static_cast<int>(speed / playerCar.GetMaxSpeed() * 8.0f) + 1;
    if (gear > 8) gear = 8;
    if (speed < 0.5f) gear = 0;
    
    speedText->setString("SPEED: " + std::to_string(speedKmh) + " km/h");
    lapText->setString("LAP: " + std::to_string(currentLap) + "/" + std::to_string(totalLaps));
    controlsText->setString("WASD: Drive | SPACE: Handbrake | R: Reset");
    gearText->setString(gear == 0 ? "N" : std::to_string(gear));
    
    fpsTimer += deltaTime;
    frameCount++;
    if (fpsTimer >= 1.0f) {
        fpsText->setString(std::to_string(frameCount) + " FPS");
        frameCount = 0;
        fpsTimer = 0.0f;
    }
    
    auto makePanel = [](float x, float y, float w, float h) {
        sf::RectangleShape panel({w, h});
        panel.setPosition({x, y});
        panel.setFillColor(sf::Color(12, 12, 20, 195));
        panel.setOutlineColor(sf::Color(204, 26, 26, 210));
        panel.setOutlineThickness(1.5f);
        return panel;
    };
    
    sf::RectangleShape speedPanel = makePanel(15.0f, 12.0f, 230.0f, 68.0f);
    sf::RectangleShape lapPanel = makePanel(static_cast<float>(screenWidth - 210), 12.0f, 195.0f, 52.0f);
    sf::RectangleShape ctrlPanel = makePanel(12.0f, static_cast<float>(screenHeight - 68), 360.0f, 56.0f);
    
    // Speed bar background
    sf::RectangleShape barBg({190.0f, 10.0f});
    barBg.setPosition({28.0f, 58.0f});
    barBg.setFillColor(sf::Color(40, 40, 45, 200));
    
    float maxKmh = playerCar.GetMaxSpeed() * 100.0f;
    float ratio = std::min(speedKmh / maxKmh, 1.0f);
    sf::RectangleShape barFill({190.0f * ratio, 10.0f});
    barFill.setPosition({28.0f, 58.0f});
    if (ratio < 0.6f) barFill.setFillColor(sf::Color(30, 180, 60));
    else if (ratio < 0.85f) barFill.setFillColor(sf::Color(220, 170, 0));
    else barFill.setFillColor(sf::Color(220, 30, 30));
    
    window.draw(speedPanel);
    window.draw(lapPanel);
    window.draw(ctrlPanel);
    window.draw(barBg);
    window.draw(barFill);
    window.draw(*speedText);
    window.draw(*lapText);
    window.draw(*controlsText);
    window.draw(*fpsText);
    window.draw(*gearText);
}
