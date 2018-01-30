#version 330

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse;

void main() {
  FragColor = texture(texture_diffuse, TexCoords);
}
