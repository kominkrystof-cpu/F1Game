#include "Graphics.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

// Camera matrices (we'll manage these globally for simplicity)
glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

// Intel UHD requires power-of-two texture dimensions.
static const int TEX_SIZE = 512;

static bool UploadTexture2D(GLuint tex, int w, int h, const unsigned char* data, bool rgba) {
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GLenum format = rgba ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);

    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "Texture upload failed (" << w << "x" << h << ")" << std::endl;
        return false;
    }
    return true;
}

static GLuint CreateTextureFromPixels(const unsigned char* data, bool rgba) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    if (tex == 0) return 0;
    if (!UploadTexture2D(tex, TEX_SIZE, TEX_SIZE, data, rgba)) {
        glDeleteTextures(1, &tex);
        return 0;
    }
    return tex;
}

static unsigned char HashNoise(int x, int y, int seed) {
    int n = x * 374761393 + y * 668265263 + seed * 982451653;
    n = (n ^ (n >> 13)) * 1274126177;
    return static_cast<unsigned char>((n ^ (n >> 16)) & 0xFF);
}

GLuint Graphics::CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexSource = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSource = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check linking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint Graphics::CreateVAO() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    return vao;
}

GLuint Graphics::CreateVBO(GLsizeiptr size, const void* data, GLenum usage) {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    return vbo;
}

GLuint Graphics::CreateEBO(GLsizeiptr size, const void* data, GLenum usage) {
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    return ebo;
}

void Graphics::DeleteVAO(GLuint vao) {
    glDeleteVertexArrays(1, &vao);
}

void Graphics::DeleteVBO(GLuint vbo) {
    glDeleteBuffers(1, &vbo);
}

void Graphics::DeleteEBO(GLuint ebo) {
    glDeleteBuffers(1, &ebo);
}

void Graphics::DeleteShaderProgram(GLuint program) {
    glDeleteProgram(program);
}

GLuint Graphics::CreateAsphaltTexture() {
    std::vector<unsigned char> data(TEX_SIZE * TEX_SIZE * 3);
    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int idx = (y * TEX_SIZE + x) * 3;
            int base = 50 + (HashNoise(x, y, 42) % 35);
            if ((x / 4 + y / 4) % 3 == 0) base += 15;
            if ((x + y * 2) % 11 == 0) base -= 10;
            if (base < 25) base = 25;
            if (base > 120) base = 120;
            data[idx]     = static_cast<unsigned char>(base);
            data[idx + 1] = static_cast<unsigned char>(base);
            data[idx + 2] = static_cast<unsigned char>(base + 4);
        }
    }
    return CreateTextureFromPixels(data.data(), false);
}

GLuint Graphics::CreateGrassTexture() {
    std::vector<unsigned char> data(TEX_SIZE * TEX_SIZE * 3);
    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int idx = (y * TEX_SIZE + x) * 3;
            int r = 18 + (HashNoise(x, y, 11) % 30);
            int g = 105 + (HashNoise(x * 2, y, 22) % 100);
            int b = 12 + (HashNoise(x, y, 33) % 22);
            if ((x / 16) % 2 == 0) { g += 30; r -= 5; }
            if ((y / 16) % 2 == 0) { g -= 10; }
            data[idx]     = static_cast<unsigned char>(r);
            data[idx + 1] = static_cast<unsigned char>(g);
            data[idx + 2] = static_cast<unsigned char>(b);
        }
    }
    return CreateTextureFromPixels(data.data(), false);
}

GLuint Graphics::CreateFenceTexture() {
    std::vector<unsigned char> data(TEX_SIZE * TEX_SIZE * 4, 0);
    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int idx = (y * TEX_SIZE + x) * 4;
            bool wire = (x % 32 < 2) || (y % 32 < 2) ||
                        (((x + y) % 48) < 2) || (((x - y + TEX_SIZE) % 48) < 2);
            if (wire) {
                data[idx] = 165; data[idx + 1] = 170; data[idx + 2] = 178; data[idx + 3] = 230;
            } else {
                data[idx + 3] = 15;
            }
        }
    }
    return CreateTextureFromPixels(data.data(), true);
}

GLuint Graphics::CreateCarbonTexture() {
    std::vector<unsigned char> data(TEX_SIZE * TEX_SIZE * 3);
    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int idx = (y * TEX_SIZE + x) * 3;
            int weave = ((x / 8) + (y / 8)) % 2;
            int base = weave ? 35 : 22;
            base += HashNoise(x, y, 99) % 12;
            data[idx] = data[idx + 1] = static_cast<unsigned char>(base);
            data[idx + 2] = static_cast<unsigned char>(base + 3);
        }
    }
    return CreateTextureFromPixels(data.data(), false);
}

void Graphics::BindTexture(GLuint tex, GLuint shaderProgram, const char* uniformName) {
    if (tex == 0) return;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    if (shaderProgram != 0 && uniformName != nullptr) {
        GLint texLoc = glGetUniformLocation(shaderProgram, uniformName);
        if (texLoc != -1) {
            glUniform1i(texLoc, 0); // Texture unit 0
        }
    }
}

void Graphics::DeleteTexture(GLuint tex) {
    if (tex != 0) glDeleteTextures(1, &tex);
}

void Graphics::SetupPerspective(int width, int height, float fov, float nearPlane, float farPlane) {
    gProjectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), nearPlane, farPlane);
}

void Graphics::SetupCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
    gViewMatrix = glm::lookAt(position, target, up);
}

void Graphics::SetupLighting(GLuint shaderProgram) {
    // Minimal fixed-function light kept for any lit geometry; track uses GL_REPLACE (no lighting).
    // Now we pass lighting parameters to shaders

    // These will be set in the rendering functions when we use the shader
    // For now, we just make sure the shader program is ready
    if (shaderProgram != 0) {
        glUseProgram(shaderProgram);

        // Set material properties (these would ideally come from material structs)
        glm::vec3 ambient(0.5f, 0.5f, 0.5f);
        glm::vec3 diffuse(0.8f, 0.8f, 0.8f);
        glm::vec3 specular(0.2f, 0.2f, 0.2f);
        float shininess = 32.0f;

        // Set light properties
        glm::vec3 lightPos(0.0f, 1.0f, 0.0f); // Directional light from above
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        glm::vec3 viewPos(0.0f, 1.25f, 0.0f); // Approximate camera position

        glUniform3fv(glGetUniformLocation(shaderProgram, "ambient"), 1, &ambient[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "diffuse"), 1, &diffuse[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "specular"), 1, &specular[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), shininess);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, &lightColor[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &viewPos[0]);

        // Fog settings
        glm::vec3 fogColor(0.68f, 0.82f, 0.94f);
        float fogDensity = 0.001f;
        glUniform3fv(glGetUniformLocation(shaderProgram, "fogColor"), 1, &fogColor[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, "fogDensity"), fogDensity);

        glUseProgram(0); // Unbind
    }
}

void Graphics::SetupFog(GLuint shaderProgram) {
    // Fog is now handled in the shader via uniforms set in SetupLighting
    // This function exists for backward compatibility
}

void Graphics::BeginTrackRender(GLuint shaderProgram /* = 0 */) {
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Use the shader program
    if (shaderProgram != 0) {
        glUseProgram(shaderProgram);

        // Set the projection and view matrices
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        if (projLoc != -1) {
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, &gProjectionMatrix[0][0]);
        }
        if (viewLoc != -1) {
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &gViewMatrix[0][0]);
        }

        // Set fog enable/disable
        GLint useFogLoc = glGetUniformLocation(shaderProgram, "useFog");
        if (useFogLoc != -1) {
            glUniform1i(useFogLoc, GL_TRUE); // Enable fog by default
        }

        // Set texture enable/disable (we'll handle this per-object)
        GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
        if (useTextureLoc != -1) {
            glUniform1i(useTextureLoc, GL_TRUE); // Enable texturing by default
        }
    }
}

void Graphics::EndTrackRender() {
    glDisable(GL_BLEND);
    glUseProgram(0); // Unbind shader program
}

void Graphics::DrawTexturedQuad3D(GLuint vao, GLuint vertexCount, GLuint shaderProgram, GLuint texture) {
    if (vao == 0 || vertexCount == 0) return;

    // Bind VAO
    glBindVertexArray(vao);

    // Bind texture if provided
    if (texture != 0 && shaderProgram != 0) {
        BindTexture(texture, shaderProgram, "textureSampler");
        GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
        if (useTextureLoc != -1) {
            glUniform1i(useTextureLoc, GL_TRUE);
        }
    } else {
        // Disable texturing
        if (shaderProgram != 0) {
            GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
            if (useTextureLoc != -1) {
                glUniform1i(useTextureLoc, GL_FALSE);
            }
        }
    }

    // Draw the vertices
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // Unbind VAO
    glBindVertexArray(0);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Graphics::DrawColoredQuad3D(GLuint vao, GLuint vertexCount, GLuint shaderProgram) {
    if (vao == 0 || vertexCount == 0) return;

    // Bind VAO
    glBindVertexArray(vao);

    // Disable texturing for colored objects
    if (shaderProgram != 0) {
        GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
        if (useTextureLoc != -1) {
            glUniform1i(useTextureLoc, GL_FALSE);
        }
    }

    // Draw the vertices
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // Unbind VAO
    glBindVertexArray(0);
}

void Graphics::SetNormal(float nx, float ny, float nz) {
    // In modern OpenGL, normals are typically part of vertex attributes
    // This function is kept for backward compatibility but does nothing
    (void)nx; (void)ny; (void)nz;
}

void Graphics::DrawSkyDome(float heading, float progress) {
    // Stub implementation for now - keep legacy behavior or implement modern version later
    // For now, we'll just do nothing to avoid breaking the build
    (void)heading; (void)progress;
}

// Backward compatibility functions (deprecated - use modern VAO-based versions)
void Graphics::DrawTexturedQuad3D(
    float x1, float y1, float z1,  // Vertex 1
    float x2, float y2, float z2,  // Vertex 2
    float x3, float y3, float z3,  // Vertex 3
    float x4, float y4, float z4,  // Vertex 4
    float u1, float v1,            // TexCoord 1
    float u2, float v2,            // TexCoord 2
    float u3, float v3,            // TexCoord 3
    float u4, float v4             // TexCoord 4
) {
    // Legacy immediate mode implementation for backward compatibility
    glBegin(GL_QUADS);
    glTexCoord2f(u1, v1); glVertex3f(x1, y1, z1);
    glTexCoord2f(u2, v2); glVertex3f(x2, y2, z2);
    glTexCoord2f(u3, v3); glVertex3f(x3, y3, z3);
    glTexCoord2f(u4, v4); glVertex3f(x4, y4, z4);
    glEnd();
}

void Graphics::DrawColoredQuad3D(
    float x1, float y1, float z1,  // Vertex 1
    float x2, float y2, float z2,  // Vertex 2
    float x3, float y3, float z3,  // Vertex 3
    float x4, float y4, float z4   // Vertex 4
) {
    // Legacy immediate mode implementation for backward compatibility
    glBegin(GL_QUADS);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glVertex3f(x4, y4, z4);
    glEnd();
}