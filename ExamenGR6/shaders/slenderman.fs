#version 330 core
out vec4 FragColor;  // El color final que se mostrará en pantalla

in vec2 TexCoords;   // Coordenadas de textura recibidas del vertex shader
in vec3 FragPos;     // Posición del fragmento en el espacio mundial
in vec3 Normal;      // Normal del fragmento en el espacio mundial

uniform sampler2D texture_diffuse1; // Mapa difuso (difuse texture)
uniform vec3 lightPos;              // Posición de la luz
uniform vec3 viewPos;               // Posición de la cámara

void main()
{
    // Cargar la textura difusa
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;

    // Iluminación básica (Phong)
    vec3 ambient = 0.1 * diffuseColor; // Componente ambiental
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0); // Producto punto entre la normal y la dirección de la luz
    vec3 diffuse = diff * diffuseColor; // Componente difusa

    // Resultado final (Combinación de luz ambiental y difusa)
    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0);
}
