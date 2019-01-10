#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
//#include "noise.glsl"
	
layout(location = 0) in vec2 inUV;
	
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

		
layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

const float EPSILON = 1.0;

float map(float value, float inMin, float inMax, float outMin, float outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

vec2 map(vec2 value, vec2 inMin, vec2 inMax, vec2 outMin, vec2 outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

vec3 map(vec3 value, vec3 inMin, vec3 inMax, vec3 outMin, vec3 outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

vec4 map(vec4 value, vec4 inMin, vec4 inMax, vec4 outMin, vec4 outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}


float sphere(in vec3 pos, in vec3 centre, float radius){
    return distance(pos, centre) - radius;
}


void scene(in vec3 pos, out float dist, out float oid){
    
    //pos.z = mod(pos.z + 50.0, 100.0) - 50.0; 
    vec3 n = normalize(pos);
    float displacement = sin(16.0 * n.x) * sin(11.0 * n.y) * sin(8.0 * n.z) * ubo.maxHeight * ubo.radius;
	float fs = sphere(pos, vec3(0.0), ubo.radius);
	float ds = fs + displacement;
	dist = ds;
	oid = 1.0;

}

float scene(in vec3 pos){
	float dist, oid;
	scene(pos, dist, oid);
	return dist;
}

void raymarch(in vec3 start, in vec3 dir, out float dist, out float oid){
    float distance_travelled = 0.0;
    const int NUM_STEPS = 4096;
    const float MIN_HIT_DISTANCE = EPSILON;
    const float MAX_TRACE_DISTANCE = 100000.0;
    
    for(int i = 0; i < NUM_STEPS; i++){
        vec3 current_pos = start + dir * distance_travelled;
        
        float dist = 0;
		float tmOid = -1;
		scene(current_pos, dist, tmOid);
        if(dist < MIN_HIT_DISTANCE){
			//dist += distance_travelled;
			oid = tmOid;
            return;
        }
        
        if(distance_travelled > MAX_TRACE_DISTANCE){
			oid = -1;
            break;
        }
        distance_travelled += dist;
    }
    dist = distance_travelled;
	oid = -1;
}

vec3 calcNormal( in vec3 p ) // for function f(p)
{
    const float h = EPSILON; // or some other value
    #define ZERO int(min(ubo.radius,0)) // or any other non constant and
	                         // cheap expression that is guaranteed
                                 // to evaluate to zero
    vec3 n = vec3(0.0);
    for( int i=ZERO; i<4; i++ )
    {
        vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
		float dist, oid;
		scene(p+e*h, dist, oid);
        n += e * dist;
    }
    return normalize(n);
}

vec3 calcNormal6( in vec3 p ) // for function f(p)
{
    const float eps = EPSILON; // or some other value
    const vec2 h = vec2(eps,0);
    return normalize( vec3(scene(p+h.xyy) - scene(p-h.xyy),
                           scene(p+h.yxy) - scene(p-h.yxy),
                           scene(p+h.yyx) - scene(p-h.yyx) ) );
}


/**
 * Return the normalized direction to march in from the eye point for a single pixel.
 * 
 * fieldOfView: vertical field of view in degrees
 * size: resolution of the output image
 * fragCoord: the x,y coordinate of the pixel in the output image
 */
vec3 rayDirection(float fieldOfView, vec2 size, vec2 fragCoord) {
    vec2 xy = fragCoord - size / 2.0;
    float z = size.y / tan(radians(fieldOfView) / 2.0);
    return normalize(vec3(xy, -z));
}

/**
 * Return a transform matrix that will transform a ray from view space
 * to world coordinates, given the eye point, the camera target, and an up vector.
 *
 * This assumes that the center of the camera is aligned with the negative z axis in
 * view space when calculating the ray marching direction. See rayDirection.
 */
mat4 viewMatrix(vec3 eye, vec3 center, vec3 up) {
    // Based on gluLookAt man page
    vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    return mat4(
        vec4(s, 0.0),
        vec4(u, 0.0),
        vec4(-f, 0.0),
        vec4(0.0, 0.0, 0.0, 1)
    );
}


vec3 lookupColour(in vec3 pos) {
	float altitude = length(pos);
	float diff = ubo.radius - altitude;
	diff /= (ubo.radius * ubo.maxHeight);
	float blue = 1.0;
	blue -= 0.5 * diff;
	float green = 0.0;
	green += diff;
	float red = 0;

	return vec3(red, blue, green);
}
  

void main()
{


    vec3 camPos = ubo.camPos.xyz;
    vec3 start = camPos;
    vec3 dir = rayDirection(45.0, ubo.viewportDim.xy, gl_FragCoord.xy);
        
    vec3 worldDir = (inverse(ubo.view) * vec4(dir, 0.0)).xyz;
    
    float dist = 0; 
	float oid = -1;

	raymarch(start, worldDir, dist, oid);
	bool hit = oid > -1;

	vec3 pos = start + worldDir * dist;
	vec3 norm = hit ? calcNormal6(pos) : vec3(0);
    vec3 colour = hit ? lookupColour(pos) : vec3(0.3);
    // Output to screen

	outAlbedo = vec4(colour, 1.0);
	//outAlbedo = vec4(vec3(0.6), 1.0);
	outNormal = vec4(norm, 1.0);
	outPosition = vec4(pos, 1.0);

}