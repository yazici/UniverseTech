#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "brdf.glsl"

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec3 inTangent;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
} ubo;

struct Light {
    vec4 position;
    vec3 color;
    float radius;
};

layout (set = 0, binding = 1) uniform UBOLIGHT
{
    Light lights[1000];
    vec4 viewPos;
    int numLights;
} uboLights;

layout (set = 0, binding = 2) uniform PER_OBJECT 
{
	mat4 model;
} ubdo;

layout (set = 1, binding = 7) uniform PER_MATERIAL
{
	vec4 baseColour;
	vec4 baseEmissive;
	vec4 baseNormal;
	float baseRoughness;
	float baseMetallic;
	float baseSpecular;
	
	uint hasTextureMap;
	uint hasNormalMap;
	uint hasRoughnessMap;
	uint hasMetallicMap;
	uint hasSpecularMap;
	uint hasEmissiveMap;
	uint hasAOMap;
	uint isVisible;
} ubmo;


layout (set = 1, binding = 0) uniform sampler2D samplerColorMap;
layout (set = 1, binding = 1) uniform sampler2D samplerNormalMap;
layout (set = 1, binding = 2) uniform sampler2D samplerRoughnessMap;
layout (set = 1, binding = 3) uniform sampler2D samplerMetallicMap;
layout (set = 1, binding = 4) uniform sampler2D samplerSpecularMap;
layout (set = 1, binding = 5) uniform sampler2D samplerEmissiveMap;
layout (set = 1, binding = 6) uniform sampler2D samplerAOMap;

layout(push_constant) uniform PushConsts {
	uint time_seconds;
	uint time_millis;
} pc;


layout (location = 0) out vec4 outFragColor;

float lambert(vec3 N, vec3 L){
  vec3 nrmN = normalize(N);
  vec3 nrmL = normalize(L);
  float result = dot(nrmN, nrmL);
  return max(result, 0.0);
}


void main() 
{

	vec3 albedo = mix(ubmo.baseColour.rgb, texture(samplerColorMap, inUV).rgb, float(ubmo.hasTextureMap));
	vec3 emissive = mix(ubmo.baseEmissive.rgb, texture(samplerEmissiveMap, inUV).rgb, float(ubmo.hasEmissiveMap));
	float roughness = mix(ubmo.baseRoughness, texture(samplerRoughnessMap, inUV).r, float(ubmo.hasRoughnessMap));
	float metallic = mix(ubmo.baseMetallic, texture(samplerMetallicMap, inUV).r, float(ubmo.hasMetallicMap));

	// Calculate normal in tangent space
	vec3 N = normalize(inNormal.xyz);
	vec3 T = normalize(inTangent.xyz);
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);
	vec3 sampledNormal = mix(ubmo.baseNormal.xyz, texture(samplerNormalMap, inUV).xyz, float(ubmo.hasNormalMap));
	vec3 tnorm = TBN * normalize(sampledNormal * 2.0 - vec3(1.0));

	N = tnorm;
	
	vec3 camPos = uboLights.viewPos.xyz;
	vec3 V = normalize(camPos - inPos.xyz);

	//float roughness = 0.3;
	//float metallic = 0.7;

	// Add striped pattern to roughness based on vertex position
#ifdef ROUGHNESS_PATTERN
	roughness = max(roughness, step(fract(inWorldPos.y * 2.02), 0.5));
#endif

	// Specular contribution
	vec3 Lo = vec3(0.0);

	for (int i = 0; i < uboLights.numLights; i++) {
	//int i = 0;
		Light light = uboLights.lights[i];
		vec4 lPos = light.position;
		vec3 L = normalize(lPos - inPos).xyz;

		float atten = light.radius / (pow(distance(lPos, inPos), 2.0) + 1.0);

		// directional lights ignore attenuation and have constant direction		
		if(light.radius < 0.001){
			L = normalize(lPos.xyz);
			atten = 1.0;
		}

		vec3 lightColour = light.color.xyz * atten;
		
		Lo += lambert(L, N) * lightColour * albedo;
		Lo += clamp(BRDF(L, V, N, lightColour, albedo, metallic, roughness), vec3(0), vec3(1));

	};

	// Combine with ambient
	vec3 color = albedo * 0.001;
	color += Lo;
	color += emissive;

	// Gamma correct
	//color = vec3(dot(N, L));
	color = pow(color, vec3(0.4545));
	//color = vec3(N);
		
	outFragColor = vec4(color, 1.0);
}