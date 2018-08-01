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

	float A = 0.1;
	float s = 5.0;
	float w = 2.0;
	float t = pc.time;

	vec3 psum = vec3(0);
	vec3 nsum = vec3(0);

	int WAVE_COUNT = 6;

	vec3 waves[6] = { 
		vec3(0, 1, 0), 
		vec3(-1, 0, 0),
		vec3(1, 1, 0), 
		vec3(19, 0, -1),
		vec3(0, -1, 12), 
		vec3(1, 27, 0)
	};

	for(int i = 0; i < WAVE_COUNT; i++){
		vec3 o = normalize(waves[i]);
		float Q = smoothstep(1 - length(dot(v, o)), 0.2, 0.4);

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