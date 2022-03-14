#version 330 core

uniform vec3 camPosition; // so we can compute the view vector
out vec4 FragColor; // the output color of this fragment

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


in vec3 P_frag; // position in world space
in vec3 N_frag; // normal in world space


void main()
{
   vec3 ambient = ambientLightColor * objAmbientReflectance * objReflectionColor;

   vec3 lightDir = normalize(light1Position - P_frag);
   float diffCoef = max(dot(N_frag, lightDir), 0.0);
   vec3 diffuse = diffCoef * light1Color * objDiffuseReflectance * objReflectionColor;

   vec3 viewDir = normalize(camPosition - P_frag);
   vec3 reflectDir = reflect(-lightDir, N_frag);
   float specCoef = pow(max(dot(viewDir, reflectDir), 0.0), objSpecularExponent);
   vec3 specular = specCoef * objSpecularReflectance * light1Color;

   float distance = length(light1Position - P_frag);
   float attenuation = max(1.0 / (attenuationC0 + attenuationC1 * distance + attenuationC2 * distance * distance), 1.0);
   vec3 diffSpecLights = (diffuse + specular) * attenuation;

   lightDir = normalize(light2Position - P_frag);
   diffCoef = max(dot(N_frag, lightDir), 0.0);
   diffuse = diffCoef * light2Color * objDiffuseReflectance * objReflectionColor;

   reflectDir = reflect(-lightDir, N_frag);
   specCoef = pow(max(dot(viewDir, reflectDir), 0.0), objSpecularExponent);
   specular = specCoef * objSpecularReflectance * light2Color;

   distance = length(light2Position - P_frag);
   attenuation = max(1.0 / (attenuationC0 + attenuationC1 * distance + attenuationC2 * distance * distance), 1.0);
   diffSpecLights += (diffuse + specular) * attenuation;

   FragColor = vec4(ambient + diffSpecLights, 1.0);
}
// you might have noticed that the shading contribution of multiple lights can fit a for loop nicely
// we will be doing that later on