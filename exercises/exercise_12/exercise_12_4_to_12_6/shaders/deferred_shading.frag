#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
   vec3 Position;
   vec3 Color;
   float Constant;
   float Linear;
   float Quadratic;
};
const int NR_LIGHTS = 128;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;
uniform float specularOffset;
uniform float lightIntensity;

uniform bool lightsAreOn;
uniform bool sharpen;
uniform bool edgeDetection;


void main()
{
   // retrieve data from gbuffer
   vec3 FragPos = texture(gPosition, TexCoords).rgb;
   vec3 Normal = texture(gNormal, TexCoords).rgb;
   vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
   float Specular = texture(gAlbedoSpec, TexCoords).a + specularOffset;

   vec2 texelSize = 1.0 / textureSize(gPosition, 0);
   vec2 up = vec2(0.0, texelSize.y);
   vec2 right = vec2(texelSize.x, 0.0);

   // Convolution kernels
   // https://en.wikipedia.org/wiki/Kernel_(image_processing)
   // sharpen filter to the diffuse and specular information of the gBuffer
   if (sharpen){
      // sharpen filter
      vec4 sharp = -texture(gAlbedoSpec, TexCoords + up).rgba
      -texture(gAlbedoSpec, TexCoords - up).rgba
      -texture(gAlbedoSpec, TexCoords + right).rgba
      -texture(gAlbedoSpec, TexCoords + right).rgba
      + 5.0 * (vec4(Diffuse, Specular - specularOffset)); // already sampled at the center

      Diffuse = sharp.rgb;
      Specular = sharp.a + specularOffset;

      /******* Method with for loops, less efficient
      Diffuse *= 5.0;
      Specular *= 5.0;
      for(int i=-1; i<2; i++) {
         for(int j=-1; j<2; j++) {
            vec2 offset = vec2(i,j) * texelSize;

            float weight = abs(i) != abs(j) ? -1.0 : 0.0;

            Diffuse += weight * texture(gAlbedoSpec, TexCoords + offset).rgb;
            Specular += weight * (texture(gAlbedoSpec, TexCoords + offset).a + specularOffset);
         }
      }*/
   }

   // edge detection filter to the diffuse and specular information of the gBuffer
   if (edgeDetection){
      // laplacian operator
      vec3 laplacian = texture(gNormal, TexCoords + up).rgb
                        + texture(gNormal, TexCoords - up).rgb
                        + texture(gNormal,TexCoords + right).rgb
                        + texture(gNormal, TexCoords - right).rgb
                        - 4.0 * Normal; // already sampled at the center

      // value in the range [0, 1], bigger values indicate less curvature
      float flatness = 1.0 - clamp(length(laplacian), 0.0, 1.0);

      Diffuse *= flatness;
      Specular *= flatness;
   }

   vec3 lighting = vec3(0,0,0);

   if (lightsAreOn){
      // calculate lighting
      lighting = Diffuse * 0.1; // hard-coded ambient component

      vec3 viewDir  = normalize(viewPos - FragPos);

      for (int i = 0; i < NR_LIGHTS; ++i)
      {
         // diffuse
         vec3 lightDir = normalize(lights[i].Position - FragPos);
         vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
         // specular
         vec3 halfwayDir = normalize(lightDir + viewDir);
         float spec = pow(max(dot(Normal, halfwayDir), 0.0), 64.0);
         vec3 specular = lights[i].Color * spec * Specular;
         // attenuation
         float distance = length(lights[i].Position - FragPos);
         float attenuation = 1.0 / (lights[i].Constant + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
         diffuse *= attenuation;
         specular *= attenuation;
         lighting += diffuse + specular;
      }

   }

   FragColor = vec4(lighting * lightIntensity , 1.0);
}

