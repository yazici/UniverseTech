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

layout (vertices = 4) out;
 
layout(location = 0) in vec3 inNormal[];
layout(location = 1) in vec3 inWorldPos[];
 
layout (location = 0) out vec3 outNormal[4];
layout (location = 1) out vec3 outWorldPos[4];
 
// Calculate the tessellation factor based on screen space
// dimensions of the edge
float screenSpaceTessFactor(vec4 p0, vec4 p1)
{
	// Calculate edge mid point
	vec4 midPoint = 0.5 * (p0 + p1);
	// Sphere radius as distance between the control points
	float radius = distance(p0, p1) / 2.0;

	// View space
	vec4 v0 = ubo.model * ubo.view * midPoint;

	// Project into clip space
	vec4 clip0 = (ubo.proj * (v0 - vec4(radius, vec3(0.0))));
	vec4 clip1 = (ubo.proj * (v0 + vec4(radius, vec3(0.0))));

	// Get normalized device coordinates
	clip0 /= clip0.w;
	clip1 /= clip1.w;

	// Convert to viewport coordinates
	clip0.xy *= ubo.viewportDim;
	clip1.xy *= ubo.viewportDim;
	
	// Return the tessellation factor based on the screen size 
	// given by the distance of the two edge control points in screen space
	// and a reference (min.) tessellation size for the edge set by the application
	return clamp(distance(clip0, clip1) / ubo.tessellatedEdgeSize * ubo.tessLevel, 1.0, 64.0);
}

// Checks the current's patch visibility against the frustum using a sphere check
// Sphere radius is given by the patch size
bool frustumCheck(float radius)
{
	// Fixed radius (increase if patch size is increased in example)
//	const float radius = 8.0f;
	vec4 pos = gl_in[gl_InvocationID].gl_Position;
	vec3 norm = normalize(pos.xyz);
	float height = ubo.radius + ubo.radius * ubo.maxHeight / 2.0;
	
	if(DISPLACEMENT_USED){	
		height = GetHeight(norm, continentTexture, ubo.radius, ubo.maxHeight);
	}

	pos = vec4(norm * height, 1.0);

	// Check sphere against frustum planes
	for (int i = 0; i < 6; i++) {
		if (dot(pos, ubo.frustumPlanes[i]) + radius < 0.0)
		{
			return false;
		}
	}
	return true;
}

void main()
{
	if (gl_InvocationID == 0)
	{
		float r = distance(gl_in[3].gl_Position, gl_in[0].gl_Position) * 2.0;
		if (!frustumCheck(r))
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;
		}
		else
		{
			if (ubo.tessLevel > 0.0)
			{
				gl_TessLevelOuter[0] = screenSpaceTessFactor(gl_in[3].gl_Position, gl_in[0].gl_Position);
				gl_TessLevelOuter[1] = screenSpaceTessFactor(gl_in[0].gl_Position, gl_in[1].gl_Position);
				gl_TessLevelOuter[2] = screenSpaceTessFactor(gl_in[1].gl_Position, gl_in[2].gl_Position);
				gl_TessLevelOuter[3] = screenSpaceTessFactor(gl_in[2].gl_Position, gl_in[3].gl_Position);
				gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
				gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
			}
			else
			{
				// Tessellation factor can be set to zero by example
				// to demonstrate a simple passthrough
				gl_TessLevelInner[0] = 1.0;
				gl_TessLevelInner[1] = 1.0;
				gl_TessLevelOuter[0] = 1.0;
				gl_TessLevelOuter[1] = 1.0;
				gl_TessLevelOuter[2] = 1.0;
				gl_TessLevelOuter[3] = 1.0;
			}
		}

	}

	gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
	outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
	outWorldPos[gl_InvocationID] = inWorldPos[gl_InvocationID];
} 
