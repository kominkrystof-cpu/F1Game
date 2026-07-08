#version 330 core
out vec4 FragColor;

// Inputs from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 aTexCoord;

// Texture sampler (if we have a texture for the car)
uniform sampler2D textureSampler;
uniform bool useTexture;

// Material properties (adjusted for car paint - more specular)
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

// Light properties
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

// Optional: simple fog (we can reuse the same fog settings as track)
uniform float fogDensity;
uniform vec3 fogColor;
uniform bool useFog;

void main()
{
    // Ambient
    vec3 ambientResult = ambient * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseResult = diff * diffuse * lightColor;

    // Specular (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), shininess);
    vec3 specularResult = spec * specular * lightColor;

    // Combine results
    vec3 result = (ambientResult + diffuseResult + specularResult);

    // Sample texture if enabled (for car, we might have a texture for paint or carbon fiber)
    vec4 texColor = texture(textureSampler, aTexCoord);
    if (useTexture) {
        result = result * texColor.rgb;
    }

    // Fog (optional)
    if (useFog) {
        float distance = length(FragPos - viewPos);
        float fogFactor = exp(-fogDensity * distance * distance);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        result = mix(result, fogColor, fogFactor);
    }

    FragColor = vec4(result, texColor.a);
}