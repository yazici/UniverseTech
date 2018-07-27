#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include "noise.glsl"
	
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
} ubo;

layout (binding = 1) uniform sampler2D continentTexture;
layout (binding = 2) uniform sampler2D terrainColorRamp;
		
layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;



vec3 lookupTerrainColor(float height){
	float mixValue = (height - ubo.radius) / (ubo.radius * ubo.maxHeight);
//	mixValue += 1.0;
//	mixValue /= 2.0;
	if(mixValue > 0.6){
		mixValue /= 1.3;
		mixValue = max(0.6, mixValue);
	}
	mixValue = clamp(mixValue, 0.48, 1.0);
	vec3 color = texture(terrainColorRamp, vec2((mixValue * (512. - 1.) + 0.5) / 512., 0.5)).rgb;
	return color;
}

void main()
{

	outAlbedo = vec4(lookupTerrainColor(length(inPosition)), 0.0);
	//outAlbedo = vec4(vec3(0.6), 1.0);
	outNormal = vec4(inNormal, 1.0);	
	outPosition = ubo.model * vec4(inPosition, 1.0);

}