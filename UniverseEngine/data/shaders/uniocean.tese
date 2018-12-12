#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

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
	vec4 frustumPlanes[6];
	vec2 viewportDim;
	float tessellatedEdgeSize;
	bool hasOcean;
} ubo;

layout(push_constant) uniform PushConsts {
	int noiseLayers;
	float time;
} pc;

layout (binding = 1) uniform sampler2D continentTexture;

layout(quads, equal_spacing, cw) in;

layout(location = 0) in vec3 inNormal[];
layout(location = 1) in vec3 inPosition[];
 
layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outWorldPos;


precision lowp float;
float PHI = 1.61803398874989484820459 * 00000.1;
float PI  = 3.14159265358979323846264 * 00000.1;
float SQ2 = 1.41421356237309504880169 * 10000.0;

float gold_noise(in vec2 coordinate, in float seed){
	return fract(tan(distance(coordinate*(seed+PHI), vec2(PHI, PI))) * SQ2);
}



void main()
{

	// Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

	//outNormal = normalize(pos.xyz);
	
	float edgeLen = length(gl_in[0].gl_Position - gl_in[1].gl_Position);
	float tF = edgeLen / gl_TessLevelOuter[0];
	tF /= 2.0;

	// Displace

	vec3 p = pos.xyz;
	vec3 norm = normalize(p);
	vec3 v = norm;
	float r = ubo.radius + ubo.radius * ubo.maxHeight / 2.0;

	float t = pc.time;

	vec3 psum = vec3(0);
	vec3 nsum = vec3(0);

	int WAVE_COUNT = 6;

	float seed = 0.23112014;

	vec3 waves[6] = { 
		vec3(gold_noise(vec2(0.3, 0.2), seed), gold_noise(vec2(0.4, 0.215), seed), gold_noise(vec2(0.778, 0.51), seed)), 
		vec3(gold_noise(vec2(0.9923, 0.2), seed), gold_noise(vec2(0.4, 0.415), seed), gold_noise(vec2(0.578, 0.91), seed)), 
		vec3(gold_noise(vec2(0.404, 0.2), seed), gold_noise(vec2(0.4, 0.815), seed), gold_noise(vec2(0.178, 0.01), seed)), 
		vec3(gold_noise(vec2(0.777, 0.2), seed), gold_noise(vec2(0.4, 0.915), seed), gold_noise(vec2(0.278, 0.61), seed)), 
		vec3(gold_noise(vec2(0.667, 0.2), seed), gold_noise(vec2(0.4, 0.715), seed), gold_noise(vec2(0.678, 0.71), seed)), 
		vec3(gold_noise(vec2(0.823, 0.2), seed), gold_noise(vec2(0.4, 0.615), seed), gold_noise(vec2(0.378, 0.11), seed))
	};

	for(int i = 0; i < WAVE_COUNT; i++){

		float A = 0.001;  // amplitude
		float s = 0.2;  // speed (km/s!)
		float w = 50.0;  // frequency

		A = A * float(i+1);
		w = w / float(i+1);

		vec3 o = normalize(waves[i]);
		float Q = smoothstep(1.0 - length(dot(v, o)), 0.01, 0.8);

		float l = dot(acos(dot(v, o)), r);
		vec3 d = cross(v, cross(v-o, v));
	
		float wavefract = w * l + s * t;

		vec3 tp = A * sin(wavefract) + Q * A * cos(wavefract) * d;
		vec3 tn = Q * A * w * sin(wavefract) - d * A * w * cos(wavefract);

		psum += tp;
		nsum += tn;
	}

	vec3 ps = v * r + v * psum;

	vec3 ns = v - v * nsum;

	pos = vec4(ps, 1.0);
	// Perspective projection
	
	gl_Position = ubo.proj * ubo.view * ubo.model * pos;


	outWorldPos = ps;
	outNormal = ns;

}