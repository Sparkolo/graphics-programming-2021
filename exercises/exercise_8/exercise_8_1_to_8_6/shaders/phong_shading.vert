#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord; // here for completness, but we are not using it just yet

uniform mat4 model; // represents model coordinates in the world coord space
uniform mat4 view;  // represents the world coordinates in the camera coord space
uniform mat4 invTranspModel; // inverse of the transpose of model (used to multiply vectors while preserving angles)
uniform mat4 projection; // camera projection matrix

out vec3 P_frag; // world space position for lighting computation
out vec3 N_frag; // world space normal for lighting computation

void main() {
   // vertex in world space (for lighting computation)
   vec4 P = model * vec4(vertex, 1.0);
   // normal in world space (for lighting computation)
   vec3 N = normalize((invTranspModel * vec4(normal, 0.0)).xyz);

   P_frag = P.xyz;
   N_frag = N;

   // final vertex position (for opengl rendering, not for lighting)
   gl_Position = projection * view * P;
}