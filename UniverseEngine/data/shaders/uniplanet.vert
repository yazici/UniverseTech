#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

	
//Patch
layout (location = 0) in vec3 pos;

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
	
float height(vec2 uv, sampler2D tex){
	float bump = texture(tex, uv).x;
	bump = clamp(bump, 0.5, 1.0);
	float offset = mix(0, ubo.maxHeight, bump);

	//return ubo.radius;

	return (ubo.radius + (ubo.radius * offset));
}


void main()
{
	//initial position
	vec3 TriPos = pos;

	vec3 n = normalize(vec3(TriPos));

	float u = atan(n.z, n.x) / (2* 3.1415926) + 0.5;
	float v = n.y * 0.5 + 0.5;
	
	vec2 uv = vec2(u, v);

	TriPos = n * height(uv, continentTexture);
	
	outNormal = normalize(pos);
	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
	outNormal = mNormal * outNormal;
	

//	outPos = (ubo.model * vec4(TriPos, 1.f)).xyz;
//	outPos.y = -outPos.y;
	outPos = TriPos;


	gl_Position = vec4(outPos, 1);
	
}

