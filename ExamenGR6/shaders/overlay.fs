#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D gameOverTexture;
uniform float alpha;

void main()
{
    vec4 texColor = texture(gameOverTexture, TexCoord);
    
    // Aplicar el efecto de alpha para crear fade in/out
    FragColor = vec4(texColor.rgb, texColor.a * alpha);
}
