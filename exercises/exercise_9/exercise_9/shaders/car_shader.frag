#version 330 core

in VS_OUT {
   vec3 Pos_eye;
   vec3 N_eye;
   vec3 Light_eye;
   vec2 textCoord;
} fs_in;

// light uniform variables
uniform vec3 ambientLightColor;
uniform vec3 lightColor;

// attenuation
uniform float attenuationC0;
uniform float attenuationC1;
uniform float attenuationC2;

// material properties
uniform float ambientOcclusionMix;
uniform float specularExponent;

// material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_ambient1;

uniform float blinn;

// output color
out vec4 FragColor;


void main()
{
   vec4 albedo = texture(texture_diffuse1, fs_in.textCoord);
   vec3 color = albedo.rgb;
   // TODO Exercise 9.5 instead of using the texture above, sample texture_normal1 for ambient and diffuse light computation
   //  (this will be the topic of our next class)

   float ambientOcclusionText = texture(texture_ambient1, fs_in.textCoord).r;
   float ambientOcclusion = mix(1.0, ambientOcclusionText, ambientOcclusionMix);

   // ambient component
   vec3 ambient = ambientLightColor * color;

   // diffuse component
   // L: vertex to light vector
   vec3 L_eye = normalize(fs_in.Light_eye - fs_in.Pos_eye).xyz;
   float diffuseModulation = max(dot(fs_in.N_eye, L_eye), 0.0);
   vec3 diffuse = lightColor * diffuseModulation * color;

   // specular component
   // R: incident light (-L) reflection vector, you can also use the reflect() function
   vec3 R_eye =  -L_eye - 2 * dot(-L_eye, fs_in.N_eye) * fs_in.N_eye;
   float specModulation = pow(max(dot(R_eye, normalize(-fs_in.Pos_eye)), 0.0), specularExponent);
   vec3 specular = lightColor * specModulation;

   // attenuation
   float dist = length(fs_in.Pos_eye - fs_in.Light_eye);
   float attenuation =  1.0 / (attenuationC0 + attenuationC1 * dist + attenuationC2 * dist * dist);

   FragColor = vec4((ambient + (diffuse + specular) * attenuation) * ambientOcclusion, albedo.a);
}