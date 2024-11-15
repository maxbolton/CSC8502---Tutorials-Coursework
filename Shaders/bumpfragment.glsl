#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform sampler2D snowDiff;
uniform sampler2D snowBump;

uniform vec3 cameraPos; // Camera Pos works
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;


uniform float dt;


in Vertex{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

vec3 fogColour = vec3(0.6f, 0.6f, 0.6f);

float camDistance;

void main(void){
	
	float fogWave = (sin(dt*0.5) + 1.0) * 0.45; // Normalized to 0-1 range

    camDistance = length(cameraPos - IN.worldPos);
    camDistance /= 100.0; // Normalizing distance

     // Adjust fog factor using fogWave with a wider scaling range (0.1 to 1.0)
    float fogFactor = clamp((100.0 - camDistance) / 100.0, 0.0, 1.0);
    fogFactor *= 0.1 + 0.9 * fogWave; // Scaling factor to allow near-clear fog at the lowest wave



	
	vec3 incident = normalize(lightPos - IN.worldPos);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec4 diffuse;
	vec3 bumpNormal;


	float verticalNorm = dot(normalize(IN.normal), vec3(0.0, 1.0, 0.0));

    bool isUp = abs(verticalNorm) > 0.5;


	//clamp 400-600
	float snowThreshold = (clamp((fogWave * 1000), 400, 1000));
	if (isUp && IN.worldPos.y > snowThreshold) {
		//fragColour.rgb = vec3(1.0, 1.0, 1.0);
		diffuse = texture(snowDiff, IN.texCoord);
		bumpNormal = texture(snowBump, IN.texCoord).rgb;
	}
	else{
		diffuse = texture(diffuseTex, IN.texCoord);
		bumpNormal = texture(bumpTex, IN.texCoord).rgb;
		
	}

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

	 if (diffuse.a <= 0.5) {
        discard;
	
    }



	// Apply the fog effect to the final fragment color
	vec4 foggedColour = mix(vec4(fogColour, 1.0), vec4(fragColour.rgb, 1.0), fogFactor);
	fragColour.rgb = foggedColour.rgb;
	

	fragColour.a = diffuse.a; // Retain the alpha value from the diffuse texture



}