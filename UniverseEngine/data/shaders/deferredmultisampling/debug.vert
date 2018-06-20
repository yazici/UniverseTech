#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
} ubo;


layout (location = 0) out vec3 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	outUV = vec3((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2, 0.0);
	gl_Position = vec4(outUV.st * 2.0f - 1.0f, 0.0f, 1.0f);
}
