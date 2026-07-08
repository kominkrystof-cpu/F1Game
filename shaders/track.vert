#version 330 core
layout (location = 0) in vec3 aPos;     // Position
layout (location = 1) in vec3 aNormal;  // Normal
layout (location = 2) in vec2 aTexCoord; // TexCoord

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}