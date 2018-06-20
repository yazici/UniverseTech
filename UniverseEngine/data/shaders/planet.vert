#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

	
//Patch
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 morph;
//Instance
layout (binding = 1) uniform PINST {
	uint level;
	vec3 a;
	vec3 r;
	vec3 s;
} pinst;

layout (binding = 0) uniform UBO {

	//Morph calculation
	vec3 camPos;
	float radius;
	float morphRange;
	float distanceLUT[32];
	//Transformation
	mat4 model;
	mat4 viewProj;
	//Height sampling
	float maxHeight;
} ubo;

layout(binding = 2) uniform sampler2D texDiffuse;
layout(binding = 3) uniform sampler2D texHeight;
layout(binding = 4) uniform sampler2D texHeightDetail;
layout(binding = 5) uniform sampler2D texDetail1;
layout(binding = 6) uniform sampler2D texDetail2;

//outputs
layout(location = 0) out vec3 Tex3;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec3 TriPos;
	
float height(vec3 Tex3)
{
	//return 0;
	vec2 uv = vec2(atan( Tex3.z, Tex3.x )/6.28318530718, acos( Tex3.y )/3.14159265359f);
	vec2 tileStretch = vec2(2, 1);
	float detail1 = (texture(texHeightDetail, uv*tileStretch*100).r)*ubo.maxHeight*0.1f;
	float detail2 = (1 - texture(texDetail2, uv*tileStretch*700).r)*0.01f;
	return texture(texHeight, uv).r*ubo.maxHeight+detail1+detail2;
}
float morphFac(float dist, int lev)
{
	float low = ubo.distanceLUT[lev-1];
	float high = ubo.distanceLUT[lev];
		
	float delta = high-low;
	float a = (dist-low)/delta;
		
	return 1 - clamp(a/ubo.morphRange, 0, 1);
}

void main()
{
	//initial position
	vec3 TriPos = pinst.a + pinst.r*pos.x + pinst.s*pos.y;
	//morph factor
	float dist = length(TriPos-ubo.camPos);
	float mPerc = morphFac(dist, int(pinst.level));
	//morph
	TriPos += mPerc*(pinst.r*morph.x + pinst.s*morph.y);
	//add height
	Tex3 = normalize(TriPos);
	TriPos = Tex3 * (ubo.radius + height(Tex3));
		
	Normal = normalize((ubo.model*vec4(Tex3, 1.0f)).xyz);
	gl_Position = ubo.viewProj * ubo.model * vec4(TriPos, 1.0f);
}