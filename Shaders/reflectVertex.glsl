#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

uniform float dt;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out Vertex{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} OUT;

void main(void){
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

	OUT.texCoord = (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;

	OUT.normal = normalize(normalMatrix * normalize(normal));




	// Calculate displacement using sin() function
	float displacement = sin(dt)* 0.025;

	// Apply displacement to the z-coordinate of the position
	vec4 displacedPosition = vec4(position.x, position.y , position.z + displacement, 1.0);

	vec4 worldPos = modelMatrix * displacedPosition;

	OUT.worldPos = worldPos.xyz;

	gl_Position = (projMatrix * viewMatrix) * worldPos;
}
