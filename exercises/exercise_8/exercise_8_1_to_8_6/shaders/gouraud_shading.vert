#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord; // here for completness, but we are not using it just yet

uniform mat4 model; // represents model coordinates in the world coord space
uniform mat4 invTransposeModel; // inverse of the transpose of  model (used to multiply vectors while preserving angles)
uniform mat4 view;  // represents the world coordinates in the camera coord space
uniform mat4 projection; // camera projection matrix
uniform vec3 camPosition; // so we can compute the view vector (could be extracted from view matrix, but let's make our life easier :) )

// send shaded color to the fragment shaders
out vec4 shadedColor;

// light uniform variables
uniform vec3 ambientLightColor;
uniform vec3 light1Position;
uniform vec3 light1Color;
uniform vec3 light2Position;
uniform vec3 light2Color;

// material properties
uniform vec3 objReflectionColor;
uniform float objAmbientReflectance;
uniform float objDiffuseReflectance;
uniform float objSpecularReflectance;
uniform float objSpecularExponent;

// attenuation uniforms
uniform float attenuationC0;
uniform float attenuationC1;
uniform float attenuationC2;


void main() {
   // vertex in world space (for light computation)
   vec4 P = model * vec4(vertex, 1.0);
   // normal in world space (for light computation)
   vec3 N = normalize((invTransposeModel * vec4(normal, 0.0)).xyz);

   // final vertex transform (for opengl rendering, not for lighting)
   gl_Position = projection * view * P;

   vec3 ambient = ambientLightColor * objAmbientReflectance * objReflectionColor;

   vec3 lightDir = normalize(light1Position - P.xyz);
   float diffCoef = max(dot(N, lightDir), 0.0);
   vec3 diffuse = diffCoef * light1Color * objDiffuseReflectance * objReflectionColor;

   vec3 viewDir = normalize(camPosition - P.xyz);
   vec3 reflectDir = reflect(-lightDir, N);
   float specCoef = pow(max(dot(viewDir, reflectDir), 0.0), objSpecularExponent);
   vec3 specular = specCoef * objSpecularReflectance * light1Color;

   float distance = length(light1Position - P.xyz);
   float attenuation = max(1.0 / (attenuationC0 + attenuationC1 * distance + attenuationC2 * distance * distance), 1.0);
   vec3 diffSpecLights = (diffuse + specular) * attenuation;

   lightDir = normalize(light2Position - P.xyz);
   diffCoef = max(dot(N, lightDir), 0.0);
   diffuse = diffCoef * light2Color * objDiffuseReflectance * objReflectionColor;

   reflectDir = reflect(-lightDir, N);
   specCoef = pow(max(dot(viewDir, reflectDir), 0.0), objSpecularExponent);
   specular = specCoef * objSpecularReflectance * light2Color;

   distance = length(light2Position - P.xyz);
   attenuation = max(1.0 / (attenuationC0 + attenuationC1 * distance + attenuationC2 * distance * distance), 1.0);
   diffSpecLights += (diffuse + specular) * attenuation;

   shadedColor = vec4(ambient + diffSpecLights, 1.0);

}