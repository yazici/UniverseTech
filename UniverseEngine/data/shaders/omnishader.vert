#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
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
	uint noise_layers;
} pc;


layout (location = 0) out vec4 outPos;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec3 outNormal;
layout (location = 4) out vec3 outTangent;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	
	
	outUV = inUV;
	outUV.t = 1.0 - outUV.t;

	vec3 outWorldPos;

	// Vertex position in world space
	outWorldPos = vec3(ubdo.model * vec4(inPos * vec3(1, -1, 1), 1));
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(ubdo.model)));
	outNormal = mNormal * normalize(inNormal);	
	outTangent = mNormal * normalize(inTangent);
	
	// Currently just vertex color
	outColor = inColor;

	gl_Position = ubo.projection * ubo.view * vec4(outWorldPos, 1);
}
