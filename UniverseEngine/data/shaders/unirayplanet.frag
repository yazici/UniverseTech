#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include "noise.glsl"
    
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

struct Light {
    vec4 position;
    vec3 color;
    float radius;
};

layout (binding = 1) uniform UBOLIGHT
{
    Light lights[1000];
    vec4 viewPos;
    int numLights;
} uboLights;

layout(push_constant) uniform PushConsts {
	uint time_seconds;
	uint time_millis;
	uint noise_layers;
} pc;
        
layout (location = 0) out vec4 outColour;

const float EPSILON = 0.01;


float sphere(in vec3 pos, in vec3 centre, float radius){
    return distance(pos, centre) - radius;
}

float planet(in vec3 pos, in vec3 centre, float radius){
    vec3 n = normalize(pos);
    float datum = sphere(pos, centre, radius);
    float d = fbm(4, n);
    d = (d + 1.0) / 2.0;
	d = clamp(d, 0, 1);
	return datum + d * ubo.radius * ubo.maxHeight;
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
    const float h = EPSILON * log2(camDist); // or some other value
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
    float diff = altitude - ubo.radius;
    diff /= (ubo.radius * ubo.maxHeight);

    diff += 1.0;

	if(diff - 0.5 < 0.001) {
		return vec3(0.0, 0.2, 0.8);
	}

	
	return vec3(0.2, 0.8, 0.2);
	

	return vec3(0.9);

    float blue = 0.0;
    blue += diff;
    float green = 1.0;
    green -= 0.8 * diff;
    float red = 0;

    return vec3(red, blue, green);
}


float softshadow( in vec3 ro, in vec3 rd, float mint, float maxt, float k )
{
    float res = 1.0;
    float ph = 1e20;
    for(float t=mint; t < maxt;)
    {
        float camDist = distance(ubo.camPos.xyz, rd*t);
        float eps = EPSILON * log2(camDist); // or some other value
        float h = scene(ro + rd*t);
        if( h < EPSILON )
            return 0.0;
        float y = h*h/(2.0*ph);
        float d = sqrt(h*h-y*y);
        res = min(res, k*d/max(0.0,t-y) );
        ph = h;
        t += h;
    }
    return res;
}


vec3 calculateLighting(in vec3 pos, in vec3 norm, in vec4 albedo)
{
    vec3 result = vec3(0.0);

    for(int i = 0; i < uboLights.numLights; ++i)
    {
        // Vector to light
        vec3 L = (uboLights.lights[i].position.xyz * vec3(1, -1, 1)) - pos;
        // Distance from light to fragment position
        float dist = length(L);

        // Viewer to fragment
        vec3 V = (uboLights.viewPos.xyz * vec3(1, -1, 1)) - pos;
        V = normalize(V);
        
        // Light to fragment
        L = normalize(L);

        // Attenuation
        float atten = uboLights.lights[i].radius / (pow(dist, 2.0) + 1.0);
        atten = 1.0;

        // Diffuse part
        vec3 N = normalize(norm);
        float NdotL = dot(N, L);

        if(NdotL < 0.0) 
            continue;

        float df = NdotL * atten;


        float camDist = distance(ubo.camPos.xyz, pos);
        float h = EPSILON * log2(camDist); // or some other value
        
        //float shadow = softshadow(pos, L, h * 2.0, dist, 128);
        vec3 diff = albedo.rgb * uboLights.lights[i].color * 4.0 * df;
        //diff *= shadow;

        // Specular part
        vec3 R = reflect(L, N);
        float NdotR = max(0.0, dot(R, V));

        float sf = pow( clamp( NdotR, 0.0, 1.0 ), 16.0) * df * (0.04 + 0.96*pow( clamp(1.0+NdotR,0.0,1.0), 5.0 ));

        vec3 spec = uboLights.lights[i].color * albedo.a * sf;

        result += diff + spec;

    }
    return result;
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

    vec3 bgc = 0.2*vec3(0.8,0.9,1.0); //*(0.5 + 0.3*worldDir.y);
    vec3 colour = bgc;

    vec3 pos = worldDir * 100000.0;
    vec3 norm = vec3(0);

    if(hit){

        pos = start + worldDir * dist;
        norm = calcNormal(pos);
        //float occ = calcAO( pos, norm );
        colour = lookupColour(pos);

        colour = calculateLighting(pos, norm, vec4(colour, 1.));

    }


    //if(hit) colour *= occ;

    //  colour = hit ? vec3(clamp(fNoise(normalize(pos) * 4) + 0.8, 0, 1)) : vec3(1.0);
    // Output to screen

    outColour = vec4(colour, 1.0);

}