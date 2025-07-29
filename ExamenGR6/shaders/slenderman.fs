#version 330 core
out vec4 FragColor;  // El color final que se mostrará en pantalla

in vec2 TexCoords;   // Coordenadas de textura recibidas del vertex shader
in vec3 FragPos;     // Posición del fragmento en el espacio mundial
in vec3 Normal;      // Normal del fragmento en el espacio mundial

uniform sampler2D texture_diffuse1; // Mapa difuso (diffuse texture)
uniform vec3 lightPos;              // Posición de la luz
uniform vec3 viewPos;               // Posición de la cámara
uniform float time;                 // Tiempo para efectos dinámicos
uniform bool isIlluminated;         // Si está siendo iluminado por la linterna

void main()
{
    // Cargar la textura difusa
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
    
    // Iluminación básica (Phong) con efectos de horror
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    if (isIlluminated) {
        // Cuando está iluminado: Más visible, colores normales, menos siniestro
        vec3 ambient = vec3(0.2, 0.2, 0.2) * diffuseColor;
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * diffuseColor * 0.8;
        
        // Efecto de "sorprendido" cuando lo iluminan
        float surprised = 1.0 + 0.1 * sin(time * 20.0);
        vec3 result = (ambient + diffuse) * surprised;
        
        FragColor = vec4(result, 1.0);
    } else {
        // Cuando NO está iluminado: Efectos de horror completos
        // Componente ambiental muy oscura y rojiza
        vec3 ambient = vec3(0.05, 0.02, 0.02) * diffuseColor;
        
        // Componente difusa con parpadeo aterrador
        float flicker = 0.7 + 0.3 * sin(time * 8.0);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * diffuseColor * flicker;
        
        // Efecto de rim lighting para silueta aterradora
        float rim = 1.0 - max(dot(viewDir, norm), 0.0);
        rim = pow(rim, 2.0);
        vec3 rimColor = vec3(0.8, 0.1, 0.1) * rim * 0.3; // Borde rojizo tenue
        
        // Resultado final combinando todos los efectos de horror
        vec3 result = ambient + diffuse + rimColor;
        
        FragColor = vec4(result, 1.0);
    }
}