#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

const int NR_LIGHTS = 15;

out VS_OUT {
    vec3 TangentViewPos;
    vec3 TangentFragPosition;
    vec3 TangentLightPos[NR_LIGHTS];
    vec3 TangentNormal;
    vec3 TangentTangent;
    vec3 TangentBitangent;
    vec2 TexCoords;
    mat3 invTBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 modelInvTra;

// light uniform variables
uniform vec3 lightPositions[NR_LIGHTS];
uniform vec3 viewPosition;

void main()
{
    vs_out.TexCoords = aTexCoords;

    vec3 T = normalize(modelInvTra * aTangent);
    vec3 N = normalize(modelInvTra * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 invTBN = mat3(T, B, N);
    mat3 TBN =  transpose(invTBN);

    for(int i = 0; i < NR_LIGHTS; ++i) {
        vs_out.TangentLightPos[i] = TBN * lightPositions[i];
    }
    vs_out.TangentViewPos = TBN * viewPosition;
    vs_out.TangentFragPosition  = TBN * vec3(model * vec4(aPos, 1.0));
    vs_out.TangentTangent = TBN * T;
    vs_out.TangentBitangent = TBN * B;
    vs_out.TangentNormal = TBN * N;
    vs_out.invTBN = invTBN;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}