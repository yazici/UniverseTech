#version 450
	
layout(location = 0) in vec3 Normal;
layout(location = 1) in vec3 WorldPos;
layout(location = 2) in vec3 vColor;
layout(location = 3) in vec3 LocalPos;
	
layout(binding = 0) uniform UBO {
	//Transformation
	mat4 model;
	mat4 view;
	mat4 proj;
	//Morph calculation
	vec4 camPos;
	float radius;
	float maxHeight;
	float maxDepth;
} ubo;

layout (binding = 1) uniform sampler2D continentTexture;
layout (binding = 2) uniform sampler2D terrainColorRamp;
		
layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

vec3 texOffset = vec3(1./1024, 0, 1./1024);

vec3 lookupTerrainColor(float height){
	float mixValue = (height - ubo.radius) / (ubo.radius * (ubo.maxHeight));
	vec3 color = texture(terrainColorRamp, vec2((mixValue * (512. - 1.) + 0.5) / 512., 0.5)).rgb;
	return color;
}
	
float height(vec2 uv, sampler2D tex){
	float bump = texture(continentTexture, uv).x;
	bump = clamp(bump, 0.5, 1.0);
	float offset = mix(0, ubo.maxHeight, bump);

	//return ubo.radius;

	return (ubo.radius + (ubo.radius * offset));
}

vec3 CalculateNormal(vec2 uv, sampler2D tex, vec3 offset)
{
	// read neighbour heights using an arbitrary small offset
	float hL = height(uv - offset.xy, tex);
	float hR = height(uv + offset.xy, tex);
	float hD = height(uv - offset.yz, tex);
	float hU = height(uv + offset.yz, tex);
	// deduce terrain normal
	vec3 N = normalize(vec3(hL - hR, hD - hU, 2.0));
	vec3 norm = normalize(Normal);
	vec3 up = vec3(0, 1, 0)-norm;
	vec3 tang = normalize(cross(norm, up)); //might need flipping
	vec3 biTan = normalize(cross(norm, tang)); //same
	mat3 localAxis = mat3(tang, biTan, norm);
	vec3 result = normalize(localAxis * normalize(N));

	result *= vec3(1, -1, 1);
	return result;
}


void main()
{
	vec3 n = normalize(vec3(WorldPos));

	float u = atan(n.z, n.x) / (2* 3.1415926) + 0.5;
	float v = n.y * 0.5 + 0.5;
	
	vec2 uv = vec2(u, v);
	
	outAlbedo = vec4(lookupTerrainColor(height(uv, continentTexture)), 1.0);
	//outAlbedo = vec4(vec3(0.6), 1.0);
	outNormal = vec4(CalculateNormal(uv, continentTexture, texOffset), 1.0);
	outPosition = vec4(WorldPos, 1.0);

} 