#version 450
	
layout(location = 0) in vec3 Normal;
layout(location = 1) in vec3 TriPos;
layout(location = 2) in vec3 vColor;
	
layout(binding = 0) uniform UBO {
	//Transformation
	mat4 model;
	mat4 view;
	mat4 proj;
	//Morph calculation
	vec3 camPos;
	double radius;
	double maxHeight;
	double maxDepth;
} ubo;
	
		
layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

	
double height(vec2 uv, sampler2D tex)
{
	return ubo.radius;
}

//vec3 CalculateNormal(vec2 uv, sampler2D tex, vec3 texOffset)
//{
//	// read neightbor heights using an arbitrary small offset
//	float hL = height(uv - texOffset.xz, tex);
//	float hR = height(uv + texOffset.xz, tex);
//	float hD = height(uv - texOffset.zy, tex);
//	float hU = height(uv + texOffset.zy, tex);
//	// deduce terrain normal
//	vec3 N = normalize(vec3(hL - hR, hD - hU, 2.0));
//	vec3 norm = normalize(Normal);
//	vec3 up = vec3(0, -1, 0)-norm;
//	vec3 tang = normalize(cross(norm, up));//might need flipping
//	vec3 biTan = normalize(cross(norm, tang));//same
//	mat3 localAxis = mat3(tang, biTan, norm);
//	return normalize(localAxis * normalize(N));
//}
//

void main()
{
	//outAlbedo = vec4(dif, 1.0);
	outAlbedo = vec4(vColor, 1.0);
	outNormal = vec4(normalize(TriPos), 1.0);
	outPosition = vec4(TriPos, 1.0);

} 