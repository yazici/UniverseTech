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

layout (binding = 1) uniform UBOLIGHT
{
    Light lights[1000];
    vec4 viewPos;
    int numLights;
} uboLights;

layout (set = 0, binding = 2) uniform PER_OBJECT 
{
	mat4 model;
} ubdo;

layout (binding = 3) uniform sampler2D samplerColorMap;
layout (binding = 4) uniform sampler2D samplerNormalMap;

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

	vec3 albedo = texture(samplerColorMap, inUV).rgb * inColor;

	// Calculate normal in tangent space
	vec3 N = normalize(inNormal);
	vec3 T = normalize(inTangent);
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);
	vec3 tnorm = TBN * normalize(texture(samplerNormalMap, inUV).xyz * 2.0 - vec3(1.0));

	N = tnorm;
	
	vec3 camPos = uboLights.viewPos.xyz;
	vec3 V = normalize(camPos - inPos.xyz);

	float roughness = 0.3;
	float metallic = 0.7;

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
		float atten = light.radius / (pow(length(V), 2.0) + 1.0);
		vec3 lightColour = light.color;// * atten;
		vec3 L = normalize(lPos - inPos).xyz;
		
		Lo += lambert(L, N) * lightColour * albedo;
		Lo += clamp(BRDF(L, V, N, lightColour, albedo, metallic, roughness), vec3(0), vec3(1));

	};

	// Combine with ambient
	vec3 color = vec3(0); //albedo * 0.02;
	color += Lo;

	// Gamma correct
	//color = vec3(dot(N, L));
	color = pow(color, vec3(0.4545));
	//color = L;
	
	
	outFragColor = vec4(color, 1.0);
}