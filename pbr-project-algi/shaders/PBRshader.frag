#version 330 core
out vec4 FragColor;

const float MIN_DOT = 0.00001;
const int NR_LIGHTS = 15;

in VS_OUT {
    vec3 TangentViewPos;
    vec3 TangentFragPosition;
    vec3 TangentLightPos[NR_LIGHTS];
    vec3 TangentNormal;
    vec3 TangentTangent;
    vec3 TangentBitangent;
    vec2 TexCoords;
    mat3 invTBN;
} fs_in;

// material uniform parameters
uniform vec3 albedoUniform;
uniform float metallicUniform;
uniform float roughnessUniform;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D heightMap;
uniform float heightScale;

// BRDF parameters
uniform bool useBeckmannDist; // Beckmann distribution vs Trowbridge-Reitz GGX Distribution
uniform bool useGGXDistApprox; // Trowbridge-Reitz GGX Distribution optimized for 16p floats
uniform bool useGGXCorrelated; // Standard Smith GGX Correlated visibility vs approximation
uniform bool useGGXCorrelateApprox; // Trowbridge-Reitz GGX Distribution optimized for 16p floats
uniform bool useFresnelRoughness; // Apply roughness to fresnel term
uniform bool useDisneyDiffuse; // Use Disney's Burley diffuse cofficient vs lambertian or OrenNayar
uniform bool useOrenNayarDiffuse; // Use Oren Nayar diffuse model vs lambertian or burley
uniform bool burleyAnisotropic; // Use burley's anisotropic BDRF (D + V)
uniform bool kullaAnisotropicRoughness; // Use Kulla vs Burley calculations for at and ab
uniform float anisotropy; // amount of anisotropy

uniform bool useMaterials; // Apply the textures from materials vs uniforms

uniform bool HDRtoneMapping; // Apply the a HDR Tonemapping
uniform bool hdrACES; // Apply the ACES Tonemapping curve vs basic Reinhard

// lights
uniform vec3 lightColors[NR_LIGHTS];

uniform vec3 viewPosition;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
float dot_clamped(vec3 A, vec3 B) {
    return max(dot(A, B), MIN_DOT);
}
// ----------------------------------------------------------------------------
vec2 ParallaxMapping(vec2 texCoords, vec3 normal, vec3 viewDir)
{
    // number of depth layers
    const float minLayers = 64;
    const float maxLayers = 256;
    float numLayers = mix(maxLayers, minLayers, abs(dot(normal, viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = 1.0 - texture(heightMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = 1.0 - texture(heightMap, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = 1.0 - texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

// ---------------------------------------------------------------------------
// Beckmann Distribution
float DistributionBeckmann(float NdotH, float roughness) {
    float cos2Alpha = NdotH * NdotH;
    float tan2Alpha = (cos2Alpha - 1.0) / cos2Alpha;
    float roughness2 = roughness * roughness;
    float denom = PI * roughness2 * cos2Alpha * cos2Alpha;
    return exp(tan2Alpha / roughness2) / denom;
}
// Trowbridge-Reitz GGX Distribution
float DistributionGGX(float NdotH, float roughness)
{
    float a = NdotH * roughness;
    float k = roughness / (1.0 - NdotH * NdotH + a * a);
    return k * k * (1.0 / PI);
}
// Trowbridge-Reitz GGX Distribution approximated
float DistributionGGX16pf(float NdotH, vec3 N, vec3 H, float roughness)
{
    vec3 NxH = cross(N, H);
    float a = NdotH * roughness;
    float k = roughness / (dot(NxH, NxH) + a * a);
    float d = k * k * (1.0 / PI);
    return min(d, 65504.0);
}


// ----------------------------------------------------------------------------
// Schlick-GGX geometry
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// Smith method for geometry with Schlick-GGX
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = dot_clamped(N, V);
    float NdotL = dot_clamped(N, L);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// Smith-GGX Correlated visibility for geometry
float VisibilitySmithGGXCorrelated(float NdotV, float NdotL, float roughness) {
    float a2 = roughness * roughness;
    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}
// Smith-GGX Correlated visibility for geometry with approximation to save 2 sqrt computations
float V_SmithGGXCorrelatedFast(float NdotV, float NdotL, float roughness) {
    float a = roughness;
    float GGXV = NdotL * (NdotV * (1.0 - a) + a);
    float GGXL = NdotV * (NdotL * (1.0 - a) + a);
    return 0.5 / (GGXV + GGXL);
}



// ----------------------------------------------------------------------------
// GGX Anisotropic Distribution
float DistributionGGX_Anisotropic(float at, float ab, vec3 N, vec3 H, vec3 T, vec3 B, float roughness)
{
    float TdotH = dot(T, H);
    float BdotH = dot(B, H);
    float NdotH = dot(N, H);

    float a2 = at * ab;
    highp vec3 v = vec3(ab * TdotH, at * BdotH, a2 * NdotH);
    highp float v2 = dot(v, v);
    float w2 = a2 / v2;
    return a2 * w2 * w2 * (1.0 / PI);
}
// Smith GGX Correlated Anisotropic visibility
float V_SmithGGXCorrelated_Anisotropic(float at, float ab, float TdotV, float BdotV, float TdotL, float BdotL, float NdotV, float NdotL) {
    float lambdaV = NdotL * length(vec3(at * TdotV, ab * BdotV, NdotV));
    float lambdaL = NdotV * length(vec3(at * TdotL, ab * BdotL, NdotL));
    float v = 0.5 / (lambdaV + lambdaL);
    return min(v, 65504.0);
}


// ----------------------------------------------------------------------------
// Fresnel term with basic Schlick approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// Fresnel term with Schlick approximation counting the roughness at the fragment
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness){
    return F0 + (max(vec3(1.0-roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}



// ----------------------------------------------------------------------------
float kD_Lambert(vec3 kS) {
    return 1.0 / PI;
}

vec3 F_Schlick(float cosTheta, vec3 F0, vec3 F90) {
    return F0 + (F90 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 kD_Burley(float NdotV, float NdotL, float LdotH, float roughness) {
    vec3 F90 = vec3(0.5 + 2.0 * roughness * LdotH * LdotH);
    vec3 lightScatter = F_Schlick(NdotL, vec3(1.0), F90);
    vec3 viewScatter = F_Schlick(NdotV, vec3(1.0), F90);
    return lightScatter * viewScatter * (1.0 / PI);
}

float kD_OrenNayar(float LdotV, float NdotL, float NdotV,float roughness) {
    float s = LdotV - NdotL * NdotV;
    float t = mix(1.0, max(NdotL, NdotV), step(0.0, s));

    float sigma2 = roughness * roughness;
    float A = 1.0 + sigma2 * (1.0 / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    return max(0.0, NdotL) * (A + B * s / t) / PI;
}



// Reinhard Tonemap
vec3 ReinhardTonemap(vec3 color) {
    return color / (color + vec3(1.0));
}
// ACES Tonemap
vec3 ACESTonemap(vec3 color) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return  clamp(((color*(a*color+b))/(color*(c*color+d)+e)), 0.0, 1.0);
}
// Gamma correction
vec3 gamma(vec3 color) {
    return pow(color, vec3(1.0/2.2));
}


void main()
{
    vec3 albedo;
    float metallic, roughness, ao;

    vec3 T = normalize(fs_in.TangentTangent);
    vec3 B = normalize(fs_in.TangentBitangent);
    vec3 N = normalize(fs_in.TangentNormal);
    vec3 V = normalize(fs_in.TangentViewPos - fs_in.TangentFragPosition);
    if(useMaterials) {

        vec2 finalCoords = fs_in.TexCoords;
        if(heightScale > 0.0) {
            float height =  texture(heightMap, fs_in.TexCoords).r;
            finalCoords = ParallaxMapping(fs_in.TexCoords, N, V);
        }

        albedo    = pow(texture(albedoMap, finalCoords).rgb, vec3(2.2));
        metallic  = texture(metallicMap, finalCoords).r;
        roughness = texture(roughnessMap, finalCoords).r;
        ao        = texture(aoMap, finalCoords).r;
        N = texture(normalMap, finalCoords).xyz;
        N = normalize(N * 2f - 1f);
    } else {
        albedo = albedoUniform;
        metallic = metallicUniform;
        roughness = roughnessUniform;
        ao = 1.0;
    }
    float NdotV = dot_clamped(N, V);

    roughness = clamp(roughness, 0.089, 1.0); // 0.089 taken from https://google.github.io/filament/Filament.md.
    roughness = roughness * roughness; // perceptually linear roughness to actual roughness

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(fs_in.TangentLightPos[i] - fs_in.TangentFragPosition);
        vec3 H = normalize(L + V);
        float NdotL = dot_clamped(N, L);
        float NdotH = dot_clamped(N, H);
        float NdotV = dot_clamped(N, V);
        float LdotH = dot_clamped(L, H);
        float LdotV = dot_clamped(L, V);
        float HdotV = dot_clamped(H, V);

        // calculate radiance based on inverse squared attenuation
        float distance = length(fs_in.TangentLightPos[i] - fs_in.TangentFragPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF, G;
        vec3 F;

        if(burleyAnisotropic) {
            float at, ab;
            if(kullaAnisotropicRoughness) {
                at = max(roughness * (1.0 + anisotropy), 0.001);
                ab = max(roughness * (1.0 - anisotropy), 0.001);
            }
            else {
                at = roughness / sqrt(1.0 - 0.9 * anisotropy);
                ab = roughness * sqrt(1.0 - 0.9 * anisotropy);
            }
            NDF = DistributionGGX_Anisotropic(at, ab, N, H, T, B, roughness);
            G = V_SmithGGXCorrelated_Anisotropic(at, ab, dot(T,V), dot(B,V), dot(T,L), dot(B,L), NdotV, NdotL);
        }
        else
        {
            if(useBeckmannDist) {
                NDF = DistributionBeckmann(NdotH, roughness);
            }
            else {
                if(useGGXDistApprox)
                NDF = DistributionGGX16pf(NdotH, N, H, roughness);
                else
                NDF = DistributionGGX(NdotH, roughness);
            }

            if(useGGXCorrelated) {
                if(useGGXCorrelateApprox)
                G = V_SmithGGXCorrelatedFast(NdotV, NdotL, roughness);
                else
                G = VisibilitySmithGGXCorrelated(NdotV, NdotL, roughness);
            }
            else {
                G = GeometrySmith(N, V, L, roughness) / (4.0 * NdotV * NdotL);
            }
        }

        if(useFresnelRoughness) {
            F = FresnelSchlickRoughness(HdotV, F0, roughness);
        }
        else {
            F = FresnelSchlick(HdotV, F0);
        }
        vec3 specular = NDF * G * F;

        vec3 diffuse = albedo * (1.0 - metallic);

        vec3 kD = vec3(1.0) - F;
        if(useDisneyDiffuse) {
            kD *= kD_Burley(NdotV, NdotL, LdotH, roughness);
        }
        else if(useOrenNayarDiffuse) {
            kD *= kD_OrenNayar(LdotV, NdotL, NdotV,roughness);
        }
        else {
            kD *= kD_Lambert(F);
        }

        // add to outgoing radiance Lo
        Lo += (kD * diffuse + specular) * radiance * NdotL;
    }

    // ambient lighting (to be replaced by IBL in future work)
    vec3 ambient = vec3(0.2) * albedo * ao;

    vec3 color = ambient + Lo;

    if(HDRtoneMapping) {
        if(hdrACES) color = ACESTonemap(color);
        else color = ReinhardTonemap(color);
        color = gamma(color);
    }

    FragColor = vec4(color, 1.0);
}