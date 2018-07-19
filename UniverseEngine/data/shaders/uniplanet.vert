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
	
} ubo;

layout (binding = 1) uniform sampler2D continentTexture;

//outputs
layout(location = 0) out vec3 Normal;
layout(location = 1) out vec3 outWorldPos;
layout(location = 2) out vec3 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};
	

void main()
{
	//initial position
	vec3 TriPos = pos;

	vec3 n = normalize(vec3(TriPos));

	float u = atan(n.z, n.x) / (2* 3.1415926) + 0.5;
	float v = n.y * 0.5 + 0.5;
	
	vec2 uv = vec2(u, v);
	
	float offset = texture(continentTexture, uv).x;

	TriPos = n * (ubo.radius + (ubo.radius * ubo.maxHeight) * offset);
	
	Normal = normalize(pos);
	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
	Normal = mNormal * Normal;
	

	outWorldPos = (ubo.model * vec4(TriPos, 1.f)).xyz;
	outWorldPos.y = -outWorldPos.y;

	outColor = vec3(1.0);  

	gl_Position = ubo.proj * ubo.view * vec4(outWorldPos, 1);
	
}

