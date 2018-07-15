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
	double radius;
	double maxHeight;
	double maxDepth;
	
} ubo;

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
	
	Normal = normalize(pos);
	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
	Normal = mNormal * Normal;
	
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(TriPos, 1.0f);

	outWorldPos = (ubo.model * vec4(TriPos, 1.f)).xyz;
	outWorldPos.y = -outWorldPos.y;

	outColor = vec3(1.0);  
	
}

