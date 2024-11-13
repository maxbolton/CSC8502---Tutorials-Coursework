#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in Vertex{
	vec2 texCoord;
	vec3 worldPos;
} IN[];

out Vertex{
	vec2 texCoord;
	vec3 worldPos;
} OUT;