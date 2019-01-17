#version 450

layout (location = 0) in vec3 inColor;
layout (location = 0) out vec4 outFragColor;

struct Light {
	vec4 position;
	vec3 color;
	float radius;
};

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
} ubo;

layout (binding = 1) uniform UBOLIGHT
{
	Light lights[1000];
	vec4 viewPos;
	int numLights;
} uboLights;


void main() 
{
  outFragColor = vec4(inColor, 1.0);
}