// shader.fs

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;     // Posición del fragmento en el espacio mundial
in vec3 Normal;      // Normal del fragmento en el espacio mundial


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_emissive1;
uniform vec3 lightPos;              // Posición de la luz
uniform vec3 viewPos;               // Posición de la cámara

uniform bool hasEmissiveMap;
uniform float time;

void main()
{
    vec3 baseColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 finalColor = baseColor * 0.05; // Fondo oscuro

    if (hasEmissiveMap)
    {
        vec3 emissiveColor = texture(texture_emissive1, TexCoords).rgb;
        float pulse = 1.0 + 0.5 * sin(time * 5.0);
        finalColor += emissiveColor * pulse;
    }
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
    // Iluminación básica (Phong)
    vec3 ambient = 0.1 * diffuseColor; // Componente ambiental
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0); // Producto punto entre la normal y la dirección de la luz
    vec3 diffuse = diff * diffuseColor; // Componente difusa
    vec3 result = ambient + diffuse;
    FragColor = vec4(finalColor, 1.0);
}