#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "noise.glsl"
	
//Patch
layout (location = 0) in vec3 inPos;

layout(binding = 0) uniform UBO {
	//Transformation
	mat4 model;
	mat4 view;
	mat4 proj;
	vec4 camPos;
	float radius;
	float maxHeight;
	float maxDepth;
	float tessLevel;
	float tessAlpha;	
} ubo;

layout (binding = 1) uniform sampler2D continentTexture;

//outputs
layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outPos;

out gl_PerVertex
{
	vec4 gl_Position;
};
	

void main()
{
	//initial position

	vec3 n = normalize(inPos);
	float height = GetHeight(inPos, continentTexture, ubo.radius, ubo.maxHeight);
	
	outNormal = n;
	outPos = n * height;

	gl_Position = vec4(outPos, 1);
	
}

