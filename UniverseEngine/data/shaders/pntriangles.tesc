#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// PN patch data
struct PnPatch
{
 float b210;
 float b120;
 float b021;
 float b012;
 float b102;
 float b201;
 float b111;
 float n110;
 float n011;
 float n101;
};

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
	vec2 viewportDim;
	float tessellatedEdgeSize;
	bool hasOcean;
} ubo;



layout(vertices=3) out;

layout(location = 0) in vec3 inNormal[];
layout(location = 1) in vec3 inWorldPos[];

layout(location = 0) out vec3 outNormal[3];
layout(location = 3) out vec3 outWorldPos[3];
layout(location = 6) out PnPatch outPatch[3];


float wij(int i, int j)
{
	return dot(gl_in[j].gl_Position.xyz - gl_in[i].gl_Position.xyz, inNormal[i]);
}

float vij(int i, int j)
{
	vec3 Pj_minus_Pi = gl_in[j].gl_Position.xyz
					- gl_in[i].gl_Position.xyz;
	vec3 Ni_plus_Nj  = inNormal[i]+inNormal[j];
	return 2.0*dot(Pj_minus_Pi, Ni_plus_Nj)/dot(Pj_minus_Pi, Pj_minus_Pi);
}

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

void main()
{
	// get data
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	outNormal[gl_InvocationID]            = inNormal[gl_InvocationID];
	outWorldPos[gl_InvocationID]            = inWorldPos[gl_InvocationID];
	//outUV[gl_InvocationID]          = inUV[gl_InvocationID];

	// set base 
	float P0 = gl_in[0].gl_Position[gl_InvocationID];
	float P1 = gl_in[1].gl_Position[gl_InvocationID];
	float P2 = gl_in[2].gl_Position[gl_InvocationID];
	float N0 = inNormal[0][gl_InvocationID];
	float N1 = inNormal[1][gl_InvocationID];
	float N2 = inNormal[2][gl_InvocationID];

	// compute control points
	outPatch[gl_InvocationID].b210 = (2.0*P0 + P1 - wij(0,1)*N0)/3.0;
	outPatch[gl_InvocationID].b120 = (2.0*P1 + P0 - wij(1,0)*N1)/3.0;
	outPatch[gl_InvocationID].b021 = (2.0*P1 + P2 - wij(1,2)*N1)/3.0;
	outPatch[gl_InvocationID].b012 = (2.0*P2 + P1 - wij(2,1)*N2)/3.0;
	outPatch[gl_InvocationID].b102 = (2.0*P2 + P0 - wij(2,0)*N2)/3.0;
	outPatch[gl_InvocationID].b201 = (2.0*P0 + P2 - wij(0,2)*N0)/3.0;
	float E = ( outPatch[gl_InvocationID].b210
			+ outPatch[gl_InvocationID].b120
			+ outPatch[gl_InvocationID].b021
			+ outPatch[gl_InvocationID].b012
			+ outPatch[gl_InvocationID].b102
			+ outPatch[gl_InvocationID].b201 ) / 6.0;
	float V = (P0 + P1 + P2)/3.0;
	outPatch[gl_InvocationID].b111 = E + (E - V)*0.5;
	outPatch[gl_InvocationID].n110 = N0+N1-vij(0,1)*(P1-P0);
	outPatch[gl_InvocationID].n011 = N1+N2-vij(1,2)*(P2-P1);
	outPatch[gl_InvocationID].n101 = N2+N0-vij(2,0)*(P0-P2);

	// set tess levels
	if (ubo.tessLevel > 0.0)
	{
		float tessLevel = max(screenSpaceTessFactor(gl_in[2].gl_Position, gl_in[0].gl_Position), max(screenSpaceTessFactor(gl_in[0].gl_Position, gl_in[1].gl_Position), screenSpaceTessFactor(gl_in[1].gl_Position, gl_in[2].gl_Position)));
	
		gl_TessLevelOuter[gl_InvocationID] = tessLevel;
		gl_TessLevelInner[0] = ubo.tessLevel;
	}
	else
	{
		// Tessellation factor can be set to zero by example
		// to demonstrate a simple passthrough
		gl_TessLevelInner[0] = 1.0;
		gl_TessLevelOuter[gl_InvocationID] = 1.0;
	}
}