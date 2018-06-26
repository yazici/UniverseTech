#version 450
	
layout(location = 0) in vec3 Tex3;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 TriPos;
layout(location = 3) in vec3 vColor;
	
layout(binding = 0) uniform UBO {
	//Transformation
	mat4 model;
	mat4 view;
	mat4 proj;
	//Morph calculation
	float distanceLUT[32];
	vec3 camPos;
	float radius;
	float morphRange;
	//Height sampling
	float maxHeight;
} ubo;
	
layout(binding = 1) uniform sampler2D texDiffuse;
layout(binding = 2) uniform sampler2D texHeight;
layout(binding = 3) uniform sampler2D texHeightDetail;
layout(binding = 4) uniform sampler2D texDetail1;
layout(binding = 5) uniform sampler2D texDetail2;

		
const vec3 texOffset = vec3(1.0f/8192.0f, 1.0f/4096.0f, 0.0f);
const vec3 texOffsetDetail = vec3(1.0f/800.0f, 1.0f/800.0f, 0.0f);

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

	
float height(vec2 uv, sampler2D tex)
{
	return texture(tex, uv).r*ubo.maxHeight;
}

vec3 CalculateNormal(vec2 uv, sampler2D tex, vec3 texOffset)
{
	// read neightbor heights using an arbitrary small offset
	float hL = height(uv - texOffset.xz, tex);
	float hR = height(uv + texOffset.xz, tex);
	float hD = height(uv - texOffset.zy, tex);
	float hU = height(uv + texOffset.zy, tex);
	// deduce terrain normal
	vec3 N = normalize(vec3(hL - hR, hD - hU, 2.0));
	vec3 norm = normalize(Normal);
	vec3 up = vec3(0, -1, 0)-norm;
	vec3 tang = normalize(cross(norm, up));//might need flipping
	vec3 biTan = normalize(cross(norm, tang));//same
	mat3 localAxis = mat3(tang, biTan, norm);
	return normalize(localAxis * normalize(N));
}

void main()
{
	vec3 tc3 = normalize(Tex3);
	vec2 uv;
	uv.x = atan( tc3.z, tc3.x );
	uv.y = acos( tc3.y );
	uv /= vec2( 2.0f * 3.14159265359f, 3.14159265359f );
	vec3 dif = texture(texDiffuse, uv).rgb;
		
	vec2 tileStretch = vec2(2, 1);
	vec2 detailUV = uv*tileStretch*100;
	vec3 detail1 = texture(texDetail1, detailUV).rgb;
	vec3 detail2 = texture(texDetail2, uv*tileStretch*700).rgb;
		
	float dist = length(TriPos-ubo.camPos);
	vec3 mix1 = detail1*dif;
	float craterAmount = 1-clamp(dif.r-0.5f, 0, 1);
	float detail = craterAmount*clamp((dist-1000)/3000, 0, 1);
	dif = mix(mix1, dif, detail);
	vec3 mix2 = detail2*dif;
	dif = mix(mix2, dif, clamp((dist-1000)/2000, 0, 1));
		
	//Calculate normal
	vec3 norm = CalculateNormal(uv, texHeight, texOffset);
	vec3 normDetail = CalculateNormal(detailUV, texHeightDetail, texOffsetDetail);
	norm = normalize(norm+normDetail*detail*2);

	//outAlbedo = vec4(dif, 1.0);
	outAlbedo = vec4(vColor, 1.0);
	outNormal = vec4(norm, 1.0);
	outPosition = vec4(TriPos, 1.0);

} 