#version 330 core
layout (location = 0) in vec3 aPos;   // Posici�n del v�rtice
layout (location = 1) in vec3 aNormal; // Normal del v�rtice
layout (location = 2) in vec2 aTexCoords; // Coordenadas de textura

out vec2 TexCoords;  // Pasar las coordenadas de textura al fragment shader
out vec3 FragPos;    // Pasar la posici�n fragmento al fragment shader
out vec3 Normal;     // Pasar la normal al fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // Calcular la posici�n del fragmento en el mundo
    Normal = mat3(transpose(inverse(model))) * aNormal; // Normal en el mundo
    TexCoords = aTexCoords;  // Pasar las coordenadas de textura

    gl_Position = projection * view * vec4(FragPos, 1.0); // Transformar la posici�n del v�rtice
}
