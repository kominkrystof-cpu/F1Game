#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

namespace Graphics {

// Modern OpenGL objects
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
GLuint CreateVAO();
GLuint CreateVBO(GLsizeiptr size, const void* data, GLenum usage = GL_STATIC_DRAW);
GLuint CreateEBO(GLsizeiptr size, const void* data, GLenum usage = GL_STATIC_DRAW);
void DeleteVAO(GLuint vao);
void DeleteVBO(GLuint vbo);
void DeleteEBO(GLuint ebo);
void DeleteShaderProgram(GLuint program);

// Texture functions (updated for modern OpenGL)
GLuint CreateAsphaltTexture();
GLuint CreateGrassTexture();
GLuint CreateFenceTexture();
GLuint CreateCarbonTexture();
void BindTexture(GLuint tex, GLuint shaderProgram = 0, const char* uniformName = "textureSampler");
void DeleteTexture(GLuint tex);

// Camera and rendering setup
void SetupPerspective(int width, int height, float fov = 72.0f,
                      float nearPlane = 0.05f, float farPlane = 2000.0f);
void SetupCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);
void SetupLighting(GLuint shaderProgram);
void SetupFog(GLuint shaderProgram);

// Rendering functions (modern OpenGL)
void BeginTrackRender(GLuint shaderProgram);
void EndTrackRender();
void DrawTexturedQuad3D(GLuint vao, GLuint vertexCount, GLuint shaderProgram,
                       GLuint texture = 0);
void DrawColoredQuad3D(GLuint vao, GLuint vertexCount, GLuint shaderProgram);

// Utility
void SetNormal(float nx, float ny, float nz);
void DrawSkyDome(float heading, float progress);

// Backward compatibility functions (deprecated - use modern VAO-based versions)
void DrawTexturedQuad3D(
    float x1, float y1, float z1,  // Vertex 1
    float x2, float y2, float z2,  // Vertex 2
    float x3, float y3, float z3,  // Vertex 3
    float x4, float y4, float z4,  // Vertex 4
    float u1, float v1,            // TexCoord 1
    float u2, float v2,            // TexCoord 2
    float u3, float v3,            // TexCoord 3
    float u4, float v4             // TexCoord 4
);

void DrawColoredQuad3D(
    float x1, float y1, float z1,  // Vertex 1
    float x2, float y2, float z2,  // Vertex 2
    float x3, float y3, float z3,  // Vertex 3
    float x4, float y4, float z4   // Vertex 4
);

} // namespace Graphics

#endif