#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform sampler2D skybox;

void main() {
    // Convert 3D direction to 2D UV coordinates for equirectangular projection
    vec2 uv = vec2(atan(TexCoords.z, TexCoords.x) / 6.28318530718 + 0.5, acos(TexCoords.y) / 3.14159265359);
    FragColor = texture(skybox, uv);
}

