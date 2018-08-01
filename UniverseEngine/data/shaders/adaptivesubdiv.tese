#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "noise.glsl"

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
	float tessLevel;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
	float tessellatedEdgeSize;
	bool hasOcean;
} ubo;

layout (binding = 1) uniform sampler2D continentTexture;

layout (constant_id = 0) const bool DISPLACEMENT_USED = true;

layout(quads, equal_spacing, cw) in;

layout(location = 0) in vec3 inNormal[];
layout(location = 1) in vec3 inPosition[];
 
layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outWorldPos;



void main()
{

	// Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

	//outNormal = normalize(pos.xyz);
	
	float edgeLen = length(gl_in[0].gl_Position - gl_in[1].gl_Position);
	float tF = edgeLen / gl_TessLevelOuter[0];
	tF /= 2.0;

	// Displace

	vec3 norm = normalize(pos.xyz);


	float height = ubo.radius + ubo.radius * ubo.maxHeight / 2.0;
	pos = vec4(norm * height, 1.0);
	
	if(DISPLACEMENT_USED){	
		height = GetHeight(norm, continentTexture, ubo.radius, ubo.maxHeight);
		pos = vec4(norm * height, 1.0);
		outNormal = CalculateNormal(pos.xyz, tF, continentTexture, ubo.radius, ubo.maxHeight);	
	}

	

	// Perspective projection
	
	gl_Position = ubo.proj * ubo.view * ubo.model * pos;


	outWorldPos = pos.xyz;

}