#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform bool flashlightOn;
uniform struct Flashlight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
} flashlight;

// Texturas del modelo
uniform sampler2D texture_diffuse1;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 color = vec3(0.0);

    // Color base del espejo (textura)
    vec3 baseColor = texture(texture_diffuse1, TexCoords).rgb;

    if (flashlightOn) {
        // Calcular dirección de la luz de la linterna
        vec3 lightDir = normalize(flashlight.position - FragPos);
        float theta = dot(lightDir, normalize(-flashlight.direction));
        float epsilon = flashlight.cutOff - flashlight.outerCutOff;
        float intensity = clamp((theta - flashlight.outerCutOff) / epsilon, 0.0, 1.0);

        // Reflexión especular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
        vec3 specular = flashlight.specular * spec * intensity;

        // Combinar color base con reflexión especular
        color = baseColor + specular;
    } else {
        // Sin linterna, mostrar solo el color base
        color = baseColor * 0.1;
    }

    FragColor = vec4(color, 1.0);
}
