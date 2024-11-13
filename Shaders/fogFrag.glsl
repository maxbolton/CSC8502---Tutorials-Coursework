#version 330 core


in Vertex{
	vec2 texCoords;
} IN;

out vec4 FragColour;


void main() {
   // vec3 color = texture(sceneTexture, texCoords).rgb;

    FragColour = vec4(0.0, 1.0, 0.0, 1.0); // Green color
}
