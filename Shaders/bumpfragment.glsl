#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform vec3 cameraPos; // Camera Pos works
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;


in Vertex{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

float fogDensity = 0.4f;
vec3 fogColour = vec3(0.5f, 0.5f, 0.5f);

float camDistance;

void main(void){

	
	camDistance = length(cameraPos - IN.worldPos);
	camDistance /= 100.0; // Normalizing distance

	float fogFactor = clamp((100.0 - camDistance) / 100.0, 0.0, 1.0);
	fogFactor = clamp(fogFactor, 0.35, 1.0);





	//fogColour = vec3(cameraPos.x, cameraPos.y, cameraPos.z);
	
	vec3 incident = normalize(lightPos - IN.worldPos);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	vec3 bumpNormal = texture(bumpTex, IN.texCoord).rgb;
	bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));

	float lambert = max(dot(incident, bumpNormal), 0.0f);
	float distance = length(lightPos - IN.worldPos);
	float attenuation = 1.0f - clamp(distance / lightRadius, 0.0f, 1.0f);

	float specFactor = clamp(dot (halfDir, bumpNormal), 0.0f, 1.0f);
	specFactor = pow(specFactor, 60.0);

	vec3 surface = (diffuse.rgb * lightColour.rgb);
	fragColour.rgb = surface * lambert * attenuation;
	fragColour.rgb += (lightColour.rgb * specFactor) * attenuation * 0.33;
	fragColour.rgb += surface * 0.1f;

	// Apply the fog effect to the final fragment color
	vec4 foggedColour = mix(vec4(fogColour, 1.0), vec4(fragColour.rgb, 1.0), fogFactor);
	fragColour.rgb = foggedColour.rgb;
	fragColour.a = diffuse.a; // Retain the alpha value from the diffuse texture



}