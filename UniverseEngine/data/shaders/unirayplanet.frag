#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include "noise.glsl"
	
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



float fNoise(in vec3 pos){
	const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

	float f = 0.0;
	vec3 q = 8.0*pos;
    f  = 0.5000*snoise( q );
    f += 0.2500*snoise( q );
    f += 0.1250*snoise( q );
    f += 0.0625*snoise( q );
	return f;
}
  



float sphere(in vec3 pos, in vec3 centre, float radius){
    return distance(pos, centre) - radius;
}

float planet(in vec3 pos, in vec3 centre, float radius){
    vec3 n = normalize(pos);
	float datum = sphere(pos, centre, radius);
	float d = fNoise(n) * ubo.radius * ubo.maxHeight;
	return datum + d;
}

void scene(in vec3 pos, out float dist, out float oid){
    
    //pos.z = mod(pos.z + 50.0, 100.0) - 50.0;
	dist = planet(pos, vec3(0), ubo.radius);
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
    const float MAX_TRACE_DISTANCE = 10000000.0;
    
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
	float camDist = distance(ubo.camPos.xyz, p);
    const float h = EPSILON * (camDist / 100.0); // or some other value
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
	float camDist = distance(ubo.camPos.xyz, p);
    const vec2 h = vec2(eps * log2(camDist),0);
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
	bool hit = oid > -0.5;

	if(!hit) discard; // ignore if no hit

	vec3 bgc = 0.1*vec3(0.8,0.9,1.0); //*(0.5 + 0.3*worldDir.y);
	vec3 colour = bgc;

	vec3 pos = worldDir * 100000.0;
	vec3 norm = vec3(0);

	if(hit){

		pos = start + worldDir * dist;
		norm = calcNormal(pos);
		//float occ = calcAO( pos, norm );
		colour = lookupColour(pos);
	}


	//if(hit) colour *= occ;

	//	colour = hit ? vec3(clamp(fNoise(normalize(pos) * 4) + 0.8, 0, 1)) : vec3(1.0);
    // Output to screen

	outAlbedo = vec4(colour, 1.0);
	//outAlbedo = vec4(vec3(0.6), 1.0);
	outNormal = vec4(norm, 1.0);
	outPosition = vec4(pos, 1.0);

}