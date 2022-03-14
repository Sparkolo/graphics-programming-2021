#version 330 core

in VS_OUT {
   vec3 CamPos_tangent;    // tangent space camera position
   vec3 Pos_tangent;       // tangent space position
   vec4 Pos_lightSpace;    // light space position
   vec3 LightDir_tangent;  // tangent space light direction
   vec3 Norm_tangent;      // tangent space normal
   vec2 textCoord;         // uv texture coordinate
   mat3 invTBN;            // matrix tha rotates from TBN to worls space
} fs_in;

// light uniform variables
uniform vec3 ambientLightColor;
uniform vec3 lightColor;
uniform vec3 lightDirection;

// material properties
uniform float ambientOcclusionMix;
uniform float normalMappingMix;
uniform float reflectionMix;
uniform float specularExponent;

// material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_ambient1;

// skybox texture
uniform samplerCube skybox;

// shadow map uniforms
uniform sampler2D shadowMap;
uniform bool softShadows;
uniform float shadowBias;

// output color
out vec4 FragColor;

float ShadowCalculation(vec4 lightSpacePos)
{
   //  lightSpacePos is in the [-1, 1] range, but textures are sampled in the [0, 1]
   vec3 lightSpaceCoord = lightSpacePos.xyz * 0.5 + vec3(0.5);
   float currentDepth = lightSpaceCoord.z;

   // fix the shadow bias based on the light incident angle
   float finalShadowBias = max((shadowBias/10.0) * (1.0 - dot(normalize(fs_in.Norm_tangent), normalize(fs_in.LightDir_tangent))), shadowBias);

   float shadow = 0.0;
   float closestDepth = -1.0;
   if(!softShadows){
      //  mind that, when you sample the shadowMap texture, the depth information is contained in the red channel
      closestDepth = texture(shadowMap, lightSpaceCoord.xy).r;
      shadow = currentDepth - finalShadowBias > closestDepth ? 1.0 : 0.0;
   }
   else {
      vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

      float weight = 0.0;
      float weightsSum = 0.0;
      for (int i=-1; i<2; i++) {
         for (int j=-1; j<2; j++) {
            closestDepth = texture(shadowMap, lightSpaceCoord.xy + vec2(i,j) * texelSize).r;

            // Setup weights for weighted average
            weight = i == 0 ? 0.5 : 0.25;
            weight += j == 0 ? 0.5 : 0.25;
            weightsSum += weight;

            shadow += currentDepth - finalShadowBias > closestDepth ? weight : 0.0;
         }
      }
      shadow /= weightsSum;
   }

   return shadow;
}

void main()
{
   // TEXTURE SAMPLING
   // diffuse texture sampling and material colors
   vec4 albedo = texture(texture_diffuse1, fs_in.textCoord);

   // ambient occlusion texture sampling and range adjustment
   float ambientOcclusion = texture(texture_ambient1, fs_in.textCoord).r;
   ambientOcclusion = mix(1.0, ambientOcclusion, ambientOcclusionMix);

   // normal texture sampling and range adjustment
   // fix normal rgb sampled range goes from [0,1] to xyz normal vector range [-1,1]
   vec3 N = texture(texture_normal1, fs_in.textCoord).rgb;
   N = normalize(N * 2.0 - 1.0);

   // mix the vertex normal and the normal map texture so we can visualize the difference with normal mapping
   N = normalize(mix(fs_in.Norm_tangent, N, normalMappingMix));


   // SKYBOX REFLECTION
   vec3 tangentIncident = (fs_in.Pos_tangent - fs_in.CamPos_tangent);
   vec3 tangentReflect = reflect(tangentIncident, N);

   //  the cube map has to be sampled with world space directions, rotate the normal so that it is in world space
   vec3 reflectionColor = texture(skybox, fs_in.invTBN * tangentReflect).rgb;


   // LIGHTING
   // ambient light
   vec3 ambient = mix(ambientLightColor, reflectionColor, reflectionMix) * albedo.rgb;

   // notice that we are now using parallel light instead of a point light
   vec3 L = normalize(-fs_in.LightDir_tangent);   // L: - light direction
   float diffuseModulation = max(dot(N, L), 0.0);
   vec3 diffuse = lightColor * diffuseModulation * albedo.rgb;

   // notice that we are now using the blinn-phong specular reflection
   vec3 V = normalize(fs_in.CamPos_tangent - fs_in.Pos_tangent); // V: surface to eye vector
   vec3 H = normalize(L + V); // H: half-vector between L and V
   float specModulation = max(dot(N, H), 0.0);
   specModulation = pow(specModulation, specularExponent);
   vec3 specular = lightColor * specModulation * reflectionColor;


   // SHADOW
   float shadow = ShadowCalculation(fs_in.Pos_lightSpace);

   FragColor = vec4(ambient * ambientOcclusion + (1.0 - shadow) * (diffuse + specular) * ambientOcclusion, albedo.a);
}