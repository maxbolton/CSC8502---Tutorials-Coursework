#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;


out Vertex{
	vec3 position;
} OUT;


void main(void){
	OUT.position = position;
	vec4 worldPos = modelMatrix * vec4(position, 1.0);
	gl_position  = projMatrix * viewMatrix * worldPos;
}