#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "noise.glsl"

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

layout (binding = 1) uniform sampler2D continentTexture;

layout(triangles, fractional_odd_spacing, ccw) in;

layout(location = 0) in vec3 inNormal[];
layout(location = 3) in vec3 inPosition[];
layout(location = 6) in PnPatch inPnPatch[];

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outPosition;


#define uvw gl_TessCoord


void main()
{
    vec3 uvwSquared = uvw * uvw;
    vec3 uvwCubed   = uvwSquared * uvw;

    // extract control points
    vec3 b210 = vec3(inPnPatch[0].b210, inPnPatch[1].b210, inPnPatch[2].b210);
    vec3 b120 = vec3(inPnPatch[0].b120, inPnPatch[1].b120, inPnPatch[2].b120);
    vec3 b021 = vec3(inPnPatch[0].b021, inPnPatch[1].b021, inPnPatch[2].b021);
    vec3 b012 = vec3(inPnPatch[0].b012, inPnPatch[1].b012, inPnPatch[2].b012);
    vec3 b102 = vec3(inPnPatch[0].b102, inPnPatch[1].b102, inPnPatch[2].b102);
    vec3 b201 = vec3(inPnPatch[0].b201, inPnPatch[1].b201, inPnPatch[2].b201);
    vec3 b111 = vec3(inPnPatch[0].b111, inPnPatch[1].b111, inPnPatch[2].b111);

    // extract control normals
    vec3 n110 = normalize(vec3(inPnPatch[0].n110, inPnPatch[1].n110, inPnPatch[2].n110));
    vec3 n011 = normalize(vec3(inPnPatch[0].n011, inPnPatch[1].n011, inPnPatch[2].n011));
    vec3 n101 = normalize(vec3(inPnPatch[0].n101, inPnPatch[1].n101, inPnPatch[2].n101));

    // compute texcoords
    //oTexCoord  = gl_TessCoord[2]*iTexCoord[0] + gl_TessCoord[0]*iTexCoord[1] + gl_TessCoord[1]*iTexCoord[2];

    // normal
    // Barycentric normal
//    vec3 barNormal = gl_TessCoord[2]*inNormal[0] + gl_TessCoord[0]*inNormal[1] + gl_TessCoord[1]*inNormal[2];
//    vec3 pnNormal  = inNormal[0]*uvwSquared[2] + inNormal[1]*uvwSquared[0] + inNormal[2]*uvwSquared[1]
//	                   + n110*uvw[2]*uvw[0] + n011*uvw[0]*uvw[1]+ n101*uvw[2]*uvw[1];
//    outNormal = ubo.tessAlpha*pnNormal + (1.0-ubo.tessAlpha) * barNormal;
	

    // compute interpolated pos
    vec3 barPos = gl_TessCoord[2]*gl_in[0].gl_Position.xyz
                + gl_TessCoord[0]*gl_in[1].gl_Position.xyz
                + gl_TessCoord[1]*gl_in[2].gl_Position.xyz;

    // save some computations
    uvwSquared *= 3.0;

    // compute PN position
    vec3 pnPos  = gl_in[0].gl_Position.xyz*uvwCubed[2]
                + gl_in[1].gl_Position.xyz*uvwCubed[0]
                + gl_in[2].gl_Position.xyz*uvwCubed[1]
                + b210*uvwSquared[2]*uvw[0]
                + b120*uvwSquared[0]*uvw[2]
                + b201*uvwSquared[2]*uvw[1]
                + b021*uvwSquared[0]*uvw[1]
                + b102*uvwSquared[1]*uvw[2]
                + b012*uvwSquared[1]*uvw[0]
                + b111*6.0*uvw[0]*uvw[1]*uvw[2];

    // final position and normal
    vec3 finalPos = (1.0-ubo.tessAlpha)*barPos + ubo.tessAlpha*pnPos;

	vec3 norm = normalize(finalPos);

	float height = GetHeight(norm, continentTexture, ubo.radius, ubo.maxHeight);

	finalPos = norm * height;
	
	outPosition = finalPos;
	outNormal = CalculateNormal(finalPos, continentTexture, ubo.radius, ubo.maxHeight);
//	
//	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
//	outNormal = mNormal * outNormal;

	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(finalPos,1.0);
}