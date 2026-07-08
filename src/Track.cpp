#include "Track.h"
#include "Graphics.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

static const float PATH_STEP = 4.0f;
static const float TEX_TILE = 8.0f;
static const int ROAD_SLICES = 160;

Track::Track() : trackWidth(20.0f), trackLength(0.0f),
                 asphaltTex(0), grassTex(0), fenceTex(0),
                 trackVAO(0), trackVBO(0), trackEBO(0), trackIndexCount(0) {
}

Track::~Track() {
    Cleanup();
}

void Track::Cleanup() {
    Graphics::DeleteTexture(asphaltTex);
    Graphics::DeleteTexture(grassTex);
    Graphics::DeleteTexture(fenceTex);
    Graphics::DeleteVAO(trackVAO);
    Graphics::DeleteVBO(trackVBO);
    Graphics::DeleteEBO(trackEBO);
    asphaltTex = grassTex = fenceTex = 0;
    trackVAO = trackVBO = trackEBO = 0;
    trackIndexCount = 0;
}

void Track::InitGraphics() {
    asphaltTex = Graphics::CreateAsphaltTexture();
    grassTex = Graphics::CreateGrassTexture();
    fenceTex = Graphics::CreateFenceTexture();
    if (asphaltTex == 0 || grassTex == 0 || fenceTex == 0) {
        std::cerr << "Warning: one or more track textures failed to create" << std::endl;
    }

    // Create track geometry for modern OpenGL rendering
    CreateTrackGeometry();
}

// Create track geometry for modern OpenGL rendering (VAO/VBO/EBO)
void Track::CreateTrackGeometry() {
    // Create a simple test quad for now - we'll replace this with actual track data later
    // Each vertex has: position (3), normal (3), texcoord (2) = 8 floats per vertex

    // Define a simple square track segment
    float vertices[] = {
        // Positions      // Normals       // TexCoords
        -10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  // Bottom-left
         10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  // Bottom-right
         10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,  // Top-right
        -10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f   // Top-left
    };

    // Define indices for two triangles (quad)
    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };

    trackIndexCount = 6;  // 6 indices for 2 triangles

    // Create and bind VAO
    trackVAO = Graphics::CreateVAO();
    glBindVertexArray(trackVAO);

    // Create and bind VBO
    trackVBO = Graphics::CreateVBO(sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and bind EBO
    trackEBO = Graphics::CreateEBO(sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord attribute (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbind VAO (safe practice)
    glBindVertexArray(0);
}

// Render track using modern OpenGL (VAO/VBO/EBO)
void Track::RenderModern(float playerProgress, float playerSpeed) {
    // Bind the track VAO and render
    glBindVertexArray(trackVAO);
    glDrawElements(GL_TRIANGLES, trackIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool Track::Load(const std::string& filename) {
    (void)filename;
    return false;
}

void Track::CreateDefaultTrack() {
    segments.clear();
    trackLength = 0.0f;
    
    segments.push_back({0.0f, 180.0f});   trackLength += 180.0f;
    segments.push_back({3.5f, 140.0f});   trackLength += 140.0f;
    segments.push_back({0.0f, 120.0f});   trackLength += 120.0f;
    segments.push_back({-3.0f, 140.0f});  trackLength += 140.0f;
    segments.push_back({0.0f, 160.0f});   trackLength += 160.0f;
    segments.push_back({-2.5f, 120.0f});  trackLength += 120.0f;
    segments.push_back({2.5f, 120.0f});   trackLength += 120.0f;
    segments.push_back({0.0f, 180.0f});   trackLength += 180.0f;
}

float Track::WrapDistance(float distance) const {
    float d = distance;
    while (d < 0.0f) d += trackLength;
    while (d >= trackLength) d -= trackLength;
    return d;
}

PathPoint Track::GetPathPoint(float distance) const {
    float d = WrapDistance(distance);
    
    float x = 0.0f, z = 0.0f, angle = 0.0f;
    float segStart = 0.0f;
    const float integrateStep = 2.0f;
    
    for (const auto& seg : segments) {
        float segEnd = segStart + seg.length;
        float curvature = seg.curve * 0.05f;
        float segLen = (d <= segEnd) ? (d - segStart) : seg.length;
        
        for (float s = 0.0f; s < segLen; s += integrateStep) {
            float ds = std::min(integrateStep, segLen - s);
            x += sinf(angle) * ds;
            z -= cosf(angle) * ds;
            angle += curvature * ds;
        }
        
        if (d <= segEnd) break;
        segStart = segEnd;
    }
    
    return {x, z, angle};
}

PathPoint Track::GetLocalSample(float distAhead, float playerProgress) const {
    PathPoint origin = GetPathPoint(playerProgress);
    PathPoint world = GetPathPoint(playerProgress + distAhead);
    
    float dx = world.x - origin.x;
    float dz = world.z - origin.z;
    float ca = cosf(origin.angle);
    float sa = sinf(origin.angle);
    
    PathPoint local;
    local.x = dx * ca + dz * sa;
    local.z = -dx * sa + dz * ca;
    local.angle = world.angle - origin.angle;
    return local;
}

float Track::GetTrackOffsetX(float progress) const {
    return GetPathPoint(progress).x;
}

float Track::GetTrackHeading(float progress) const {
    return GetPathPoint(progress).angle;
}

void Track::DrawGround(float playerProgress, GLuint shaderProgram) {
    Graphics::BindTexture(grassTex, shaderProgram);
    const float cell = 50.0f;
    const float texScroll = playerProgress / TEX_TILE;

    for (int gx = -14; gx < 14; gx++) {
        for (int gz = -1; gz < 40; gz++) {
            // Isolate matrix for each ground cell to prevent transformation leakage
            glPushMatrix();

            float x1 = gx * cell;
            float x2 = x1 + cell;
            float z1 = -gz * cell;
            float z2 = z1 - cell;
            float u0 = static_cast<float>(gx) * 0.5f;
            float v0 = texScroll + static_cast<float>(gz) * (cell / TEX_TILE);
            float v1 = v0 + cell / TEX_TILE;
            Graphics::DrawTexturedQuad3D(
                x1, -0.01f, z1, x2, -0.01f, z1, x2, -0.01f, z2, x1, -0.01f, z2,
                u0, v0, u0 + 0.5f, v0, u0 + 0.5f, v1, u0, v1);

            // Restore matrix state
            glPopMatrix();
        }
    }
}

void Track::DrawRoadStrip(float playerProgress, GLuint shaderProgram) {
    const float curbWidth = 2.5f;
    const float curbHeight = 0.15f;
    const float uRoad = (trackWidth * 2.0f) / TEX_TILE;
    const float texScroll = playerProgress / TEX_TILE;

    Graphics::BindTexture(asphaltTex, shaderProgram);
    PathPoint carPos = GetPathPoint(playerProgress);
    float carAngle = carPos.angle;

    for (int i = 0; i < ROAD_SLICES; i++) {
        // Isolate matrix for each road segment to prevent transformation leakage
        glPushMatrix();

        float d0 = PATH_STEP + i * PATH_STEP;
        float d1 = PATH_STEP + (i + 1) * PATH_STEP;
        PathPoint p0 = GetLocalSample(d0, playerProgress);
        PathPoint p1 = GetLocalSample(d1, playerProgress);

        // Get absolute track angles (not relative to car)
        float trackAngle0 = p0.angle + carAngle;
        float trackAngle1 = p1.angle + carAngle;
        float cos0 = cosf(trackAngle0), sin0 = sinf(trackAngle0);
        float cos1 = cosf(trackAngle1), sin1 = sinf(trackAngle1);

        float lxL0 = p0.x - cos0 * trackWidth;
        float lzL0 = p0.z - sin0 * trackWidth;
        float lxR0 = p0.x + cos0 * trackWidth;
        float lzR0 = p0.z + sin0 * trackWidth;
        float lxL1 = p1.x - cos1 * trackWidth;
        float lzL1 = p1.z - sin1 * trackWidth;
        float lxR1 = p1.x + cos1 * trackWidth;
        float lzR1 = p1.z + sin1 * trackWidth;

        float v0 = texScroll + d0 / TEX_TILE;
        float v1 = texScroll + d1 / TEX_TILE;

        Graphics::DrawTexturedQuad3D(
            lxL0, 0.03f, lzL0, lxR0, 0.03f, lzR0, lxR1, 0.03f, lzR1, lxL1, 0.03f, lzL1,
            0.0f, v0, uRoad, v0, uRoad, v1, 0.0f, v1);

        // White edge lines (flat color, no texture)
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.95f, 0.95f, 0.95f);
        float lineW = 0.35f;
        float offInner = trackWidth - 0.5f;
        float offOuter = offInner - lineW;

        auto edgeLine = [&](float sign) {
            float ax0 = p0.x + sign * cos0 * offInner;
            float az0 = p0.z + sign * sin0 * offInner;
            float bx0 = p0.x + sign * cos0 * offOuter;
            float bz0 = p0.z + sign * sin0 * offOuter;
            float ax1 = p1.x + sign * cos1 * offInner;
            float az1 = p1.z + sign * sin1 * offInner;
            float bx1 = p1.x + sign * cos1 * offOuter;
            float bz1 = p1.z + sign * sin1 * offOuter;
            Graphics::DrawColoredQuad3D(ax0, 0.04f, az0, bx0, 0.04f, bz0, bx1, 0.04f, bz1, ax1, 0.04f, az1);
        };
        edgeLine(-1.0f);
        edgeLine(1.0f);

        // Dashed center line
        if (i % 2 == 0) {
            glColor3f(0.92f, 0.92f, 0.88f);
            float hw = 0.18f;
            Graphics::DrawColoredQuad3D(
                p0.x - hw, 0.045f, p0.z, p0.x + hw, 0.045f, p0.z,
                p1.x + hw, 0.045f, p1.z, p1.x - hw, 0.045f, p1.z);
        }

        // Curbs
        for (int side = 0; side < 2; side++) {
            float sign = (side == 0) ? -1.0f : 1.0f;
            bool red = ((i + side) % 2 == 0);

            float lix0 = p0.x + sign * cos0 * trackWidth;
            float liz0 = p0.z + sign * sin0 * trackWidth;
            float lox0 = p0.x + sign * cos0 * (trackWidth + curbWidth);
            float loz0 = p0.z + sign * sin0 * (trackWidth + curbWidth);
            float lix1 = p1.x + sign * cos1 * trackWidth;
            float liz1 = p1.z + sign * sin1 * trackWidth;
            float lox1 = p1.x + sign * cos1 * (trackWidth + curbWidth);
            float loz1 = p1.z + sign * sin1 * (trackWidth + curbWidth);

            if (red) glColor3f(0.85f, 0.08f, 0.08f);
            else glColor3f(0.95f, 0.95f, 0.95f);
            Graphics::DrawColoredQuad3D(lox0, curbHeight, loz0, lix0, curbHeight, liz0,
                                        lix1, curbHeight, liz1, lox1, curbHeight, loz1);

            glColor3f(red ? 0.55f : 0.65f, red ? 0.04f : 0.65f, red ? 0.04f : 0.65f);
            glBegin(GL_QUADS);
            glVertex3f(lox0, 0.0f, loz0);
            glVertex3f(lox0, curbHeight, loz0);
            glVertex3f(lox1, curbHeight, loz1);
            glVertex3f(lox1, 0.0f, loz1);
            glEnd();
        }

        Graphics::BindTexture(asphaltTex, shaderProgram);

        // Restore matrix state
        glPopMatrix();
    }
}

void Track::DrawTrackEdgeStrip(float playerProgress, bool leftSide, bool isFence, GLuint shaderProgram) {
    const float edgeOffset = trackWidth + (isFence ? 8.0f : 5.5f);
    const float sign = leftSide ? -1.0f : 1.0f;
    const float texScroll = playerProgress / TEX_TILE;
    PathPoint carPos = GetPathPoint(playerProgress);
    float carAngle = carPos.angle;

    if (isFence) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Graphics::BindTexture(fenceTex, shaderProgram);
    }

    for (int i = 0; i < ROAD_SLICES; i++) {
        // Isolate matrix for each track edge segment to prevent transformation leakage
        glPushMatrix();

        float d0 = PATH_STEP + i * PATH_STEP;
        float d1 = PATH_STEP + (i + 1) * PATH_STEP;
        PathPoint p0 = GetLocalSample(d0, playerProgress);
        PathPoint p1 = GetLocalSample(d1, playerProgress);

        // Get absolute track angles (not relative to car)
        float trackAngle0 = p0.angle + carAngle;
        float trackAngle1 = p1.angle + carAngle;
        float cos0 = cosf(trackAngle0), sin0 = sinf(trackAngle0);
        float cos1 = cosf(trackAngle1), sin1 = sinf(trackAngle1);

        float lx0 = p0.x + sign * cos0 * edgeOffset;
        float lz0 = p0.z + sign * sin0 * edgeOffset;
        float lx1 = p1.x + sign * cos1 * edgeOffset;
        float lz1 = p1.z + sign * sin1 * edgeOffset;

        if (isFence) {
            float v0 = texScroll + d0 / 4.0f;
            float v1 = texScroll + d1 / 4.0f;
            Graphics::DrawTexturedQuad3D(
                lx0, 0.0f, lz0, lx0, 3.0f, lz0, lx1, 3.0f, lz1, lx1, 0.0f, lz1,
                0.0f, v0, 0.0f, v0 + 0.75f, 0.0f, v1 + 0.75f, 0.0f, v1);

            if (i % 4 == 0) {
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                glColor3f(0.45f, 0.45f, 0.48f);
                float pw = 0.3f;
                glBegin(GL_QUADS);
                glVertex3f(lx0 - pw, 0.0f, lz0);
                glVertex3f(lx0 + pw, 0.0f, lz0);
                glVertex3f(lx0 + pw, 3.5f, lz0);
                glVertex3f(lx0 - pw, 3.5f, lz0);
                glEnd();
                glEnable(GL_BLEND);
                Graphics::BindTexture(fenceTex, shaderProgram);
            }
        } else {
            float h = 1.1f;
            glColor3f(0.72f, 0.72f, 0.75f);
            Graphics::DrawColoredQuad3D(lx0, 0.0f, lz0, lx0, h, lz0, lx1, h, lz1, lx1, 0.0f, lz1);
            glColor3f(0.88f, 0.88f, 0.90f);
            Graphics::DrawColoredQuad3D(
                lx0 - 0.25f, h, lz0, lx0 + 0.25f, h, lz0,
                lx1 + 0.25f, h, lz1, lx1 - 0.25f, h, lz1);
        }

        // Restore matrix state
        glPopMatrix();
    }

    if (isFence) {
        glDisable(GL_BLEND);
    }
}

void Track::DrawGrandstands(float playerProgress, GLuint shaderProgram) {
    const float standOffsets[] = {80.0f, 160.0f, 260.0f, 380.0f, 500.0f};
    PathPoint carPos = GetPathPoint(playerProgress);
    float carAngle = carPos.angle;

    for (float offset : standOffsets) {
        // Isolate matrix for each grandstand to prevent transformation leakage
        glPushMatrix();

        PathPoint sp = GetLocalSample(offset, playerProgress);
        // Get absolute track angle (not relative to car)
        float trackAngle = sp.angle + carAngle;
        float cosA = cosf(trackAngle), sinA = sinf(trackAngle);

        float cx = sp.x + cosA * (trackWidth + 30.0f);
        float cz = sp.z + sinA * (trackWidth + 30.0f);
        float perpCos = -sinA, perpSin = cosA;
        float halfW = 15.0f, halfD = 7.0f;

        float lx[4], lz[4];
        float wx[4] = {
            cx - perpCos * halfW - cosA * halfD, cx + perpCos * halfW - cosA * halfD,
            cx + perpCos * halfW + cosA * halfD, cx - perpCos * halfW + cosA * halfD
        };
        float wz[4] = {
            cz - perpSin * halfW - sinA * halfD, cz + perpSin * halfW - sinA * halfD,
            cz + perpSin * halfW + sinA * halfD, cz - perpSin * halfW + sinA * halfD
        };
        for (int c = 0; c < 4; c++) { lx[c] = wx[c]; lz[c] = wz[c]; }

        glColor3f(0.55f, 0.55f, 0.58f);
        Graphics::DrawColoredQuad3D(lx[0], 0.0f, lz[0], lx[1], 0.0f, lz[1], lx[2], 0.0f, lz[2], lx[3], 0.0f, lz[3]);

        for (int row = 0; row < 3; row++) {
            float rowH = 1.5f + row * 2.0f;
            glColor3f(0.38f + row * 0.04f, 0.38f + row * 0.04f, 0.40f);
            Graphics::DrawColoredQuad3D(
                lx[0], rowH, lz[0], lx[1], rowH, lz[1],
                lx[1], rowH + 1.4f, lz[1], lx[0], rowH + 1.4f, lz[0]);

            glDisable(GL_LIGHTING);
            glPointSize(3.5f);
            for (int c = 0; c < 30; c++) {
                float t = static_cast<float>(c) / 29.0f;
                float hue = static_cast<float>((c * 41 + row * 17) % 100) / 100.0f;
                glColor3f(0.4f + hue * 0.5f, 0.25f + (1.0f - hue) * 0.35f, 0.3f);
                glBegin(GL_POINTS);
                glVertex3f(lx[0] + (lx[1] - lx[0]) * t, rowH + 1.5f, lz[0] + (lz[1] - lz[0]) * t);
                glEnd();
            }
        }

        glColor3f(0.22f, 0.22f, 0.25f);
        Graphics::DrawColoredQuad3D(lx[0], 7.5f, lz[0], lx[1], 7.5f, lz[1], lx[3], 7.5f, lz[3], lx[2], 7.5f, lz[2]);

        // Restore matrix state
        glPopMatrix();
    }
}

void Track::Render(float playerProgress, float playerSpeed) {
    // Use modern OpenGL rendering
    RenderModern(playerProgress, playerSpeed);
}

void Track::Draw3D(float playerProgress, float playerSpeed, GLuint shaderProgram) {
    (void)playerSpeed;

    DrawGround(playerProgress, shaderProgram);
    DrawRoadStrip(playerProgress, shaderProgram);
    DrawTrackEdgeStrip(playerProgress, true, false, shaderProgram);
    DrawTrackEdgeStrip(playerProgress, false, false, shaderProgram);
    DrawTrackEdgeStrip(playerProgress, true, true, shaderProgram);
    DrawGrandstands(playerProgress, shaderProgram);
}

bool Track::CheckCollision(float posX, float posZ, float carWidth, float carLength) {
    (void)carWidth;
    (void)carLength;
    float dx = GetTrackOffsetX(posZ);
    if (posX < dx - trackWidth || posX > dx + trackWidth) return true;
    return false;
}
