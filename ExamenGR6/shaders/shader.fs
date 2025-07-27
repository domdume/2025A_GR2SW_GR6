#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;     // Posición del fragmento en el espacio mundial
in vec3 Normal;      // Normal del fragmento en el espacio mundial

struct Material {
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
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
};

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_emissive1;
uniform vec3 viewPos;               // Posición de la cámara

uniform Material material;
uniform PointLight light;
uniform SpotLight flashlight;
uniform bool flashlightOn;
uniform bool hasEmissiveMap;
uniform float time;

// Función para calcular iluminación de punto con efecto disco
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading más sutil para ambiente de disco
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 0.5);
    
    // Attenuation más fuerte para crear zonas más oscuras
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance * 1.5 + light.quadratic * (distance * distance));
    
    // Efecto pulsante para las luces de disco
    float pulse = 0.7 + 0.3 * sin(time * 2.0 + distance * 0.1);
    
    // Combine results con menos ambient para ambiente más oscuro
    vec3 ambient = light.ambient * diffuseColor * 0.3;
    vec3 diffuse = light.diffuse * diff * diffuseColor * pulse;
    vec3 specular = light.specular * spec * diffuseColor * 0.5;
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

// Función para calcular iluminación de linterna (spotlight)
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // Combine results
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec * diffuseColor;
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return (ambient + diffuse + specular);
}

void main()
{
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Base muy oscura para ambiente de discoteca
    vec3 result = vec3(0.02, 0.02, 0.05) * diffuseColor;
    
    // Iluminación principal más tenue
    result += CalcPointLight(light, norm, FragPos, viewDir, diffuseColor) * 0.6;
    
    // Agregar linterna si está activada (más brillante para contraste)
    if (flashlightOn) {
        result += CalcSpotLight(flashlight, norm, FragPos, viewDir, diffuseColor);
    }
    
    // Efecto emissive más dramático para luces de disco
    if (hasEmissiveMap) {
        vec3 emissiveColor = texture(texture_emissive1, TexCoords).rgb;
        // Múltiples pulsos con diferentes frecuencias para efecto disco
        float pulse1 = 0.5 + 0.5 * sin(time * 3.0);
        float pulse2 = 0.3 + 0.7 * sin(time * 1.5 + 1.57);
        float finalPulse = pulse1 * pulse2;
        
        // Colores más vibrantes para las luces emissive
        vec3 discoColors = emissiveColor;
        discoColors.r *= 1.0 + 0.5 * sin(time * 2.0);
        discoColors.g *= 1.0 + 0.5 * sin(time * 2.5 + 2.0);
        discoColors.b *= 1.0 + 0.5 * sin(time * 1.8 + 4.0);
        
        result += discoColors * finalPulse * 1.2;
    }
    
    FragColor = vec4(result, 1.0);
}