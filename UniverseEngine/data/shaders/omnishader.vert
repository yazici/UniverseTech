#version 450

layout (location = 0) out vec3 outColour;

void main(){
	outColour = vec3(1, 0, 0);
	vec2 UV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(UV * 2.0f + -1.0f, 0.0f, 1.0f);
}