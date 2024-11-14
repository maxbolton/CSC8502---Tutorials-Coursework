#version 330 core

uniform sampler2D diffuseTex;
uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

in Vertex {
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
} IN;

out vec4 fragColour;

float fogDensity = 0.1f;
vec3 fogColour = vec3(0.5f, 0.5f, 0.5f);
float camDistance;

void main(void) {
    
    camDistance = length(cameraPos - IN.worldPos);
    camDistance /= 100.0; // Normalizing distance

	float fogFactor = clamp((100.0 - camDistance) / 100.0, 0.0, 1.0);

    fogFactor = clamp(fogFactor, 0.35, 1.0);


    /* Debugging: Output camDistance and fogFactor to the color channels
    fragColour = vec4(camDistance / 100.0, fogFactor, 0.0, 1.0);
    return;*/

    vec3 incident = normalize(lightPos - IN.worldPos);
    vec3 viewDir = normalize(cameraPos - IN.worldPos);
    vec3 halfDir = normalize(incident + viewDir);

    vec4 diffuse = texture(diffuseTex, IN.texCoord);

    float lambert = max(dot(incident, IN.normal), 0.0f);
    float distance = length(lightPos - IN.worldPos);
    float attenuation = 1.0 - clamp(distance / lightRadius, 0.0, 1.0);

    float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
    specFactor = pow(specFactor, 60.0);

    vec3 surface = (diffuse.rgb * lightColour.rgb);
    fragColour.rgb = surface * lambert * attenuation;
    fragColour.rgb += (lightColour.rgb * specFactor) * attenuation * 0.33;
    fragColour.rgb += surface * 0.1f; // ambient!



    vec4 foggedColour = mix(vec4(fogColour, 1.0), vec4(fragColour.rgb, 1.0), fogFactor);
	fragColour.rgb = foggedColour.rgb;
    fragColour.a = diffuse.a;
}
