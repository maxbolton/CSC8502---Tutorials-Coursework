#version 330 core


in vec2 texCoords;
in vec3 position;


out Vertex{
	vec2 texCoords;
} OUT;


uniform sampler2D sceneTexture;


void main() {
    gl_Position = vec4(position, 1);
    OUT.texCoords = texCoords;
    //FragColor = vec4(color, 1.0); // Display color texture
}
