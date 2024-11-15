#version 330 core

uniform sampler2D diffuseTex;
uniform samplerCube cubeTex;

uniform float dt;

uniform vec3 cameraPos;

in Vertex{
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
} IN;

out vec4 fragColour;

vec3 fogColour = vec3(0.5f, 0.5f, 0.5f);

float camDistance;

void main(void){
    // Calculate the fog wave to modulate the fog factor
    float fogWave = (sin(dt*0.5) + 1.0) * 0.45; // Normalized to 0-1 range

    camDistance = length(cameraPos - IN.worldPos);
    camDistance /= 100.0; // Normalizing distance

    // Adjust fog factor using fogWave without overpowering the final color
    float fogFactor = clamp((100.0 - camDistance) / 100.0, 0.0, 1.0);
    fogFactor *= 0.5 + 0.5 * fogWave; // Scaling factor to modulate fog intensity

    vec4 diffuse = texture(diffuseTex, IN.texCoord);
    vec3 viewDir = normalize(cameraPos - IN.worldPos);

    vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
    vec4 reflectTex = texture(cubeTex, reflectDir);

    fragColour = reflectTex + (diffuse * 0.25f);

    // Apply the modulated fog effect to the final fragment color
    vec4 foggedColour = mix(vec4(fogColour, 1.0), vec4(fragColour.rgb, 1.0), fogFactor);

    fragColour.rgb = foggedColour.rgb;
}
