#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

	
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPosition;
	
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
	float tessAlpha;
	bool hasOcean;
} ubo;

		
layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;


void main()
{

	outAlbedo = vec4(0.2, 0.3, 0.9, 0.8);
	//outAlbedo = vec4(vec3(0.6), 1.0);
	outNormal = vec4(inNormal, 1.0);	
	outPosition = ubo.model * vec4(inPosition, 1.0);

}