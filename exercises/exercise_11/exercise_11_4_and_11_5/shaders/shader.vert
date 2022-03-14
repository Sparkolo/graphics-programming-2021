#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT {
   vec3 CamPos_tangent;
   vec3 Pos_tangent;
   vec3 LightDir_tangent;
   vec3 Norm_tangent;
   vec2 textCoord;
   mat3 invTBN;
} vs_out;

// transformations
uniform mat4 projection;   // camera projection matrix
uniform mat4 view;         // represents the world in the eye coord space
uniform mat4 model;        // represents model in the world coord space
uniform mat3 modelInvTra;  // inverse of the transpose of the model matrix, used to rotate vectors while preserving angles

// light uniform variables
uniform vec3 lightDirection;
uniform vec3 viewPosition;


void main() {
   // send text coord to fragment shaders
   vs_out.textCoord = textCoord;

   // TODO exercise 10.4 compute the TBN matrix, which maps from world space to Tangent space
   vec3 N = normalize(modelInvTra * normal); // vertex normal in world space
   vec3 T = normalize(modelInvTra * tangent);
   // T = normalize(T - dot(T, N) * N); // this would allow me to know that T is laying in a plane with no N component for sure
   vec3 B = normalize(modelInvTra * bitangent);
   //vec3 B = cross(T, N); // this would spare me using another matrix multiplication if T and B are perpendicular already
   mat3 TBN = transpose(mat3(T, B, N)); // transpose to invert the matrix (from tangent->world to world->tangent)

   // variables we wanna send to the fragment shaders
   // inverse of TBN, to map from tangent space to world space (needed for reflections)
   vs_out.invTBN = transpose(TBN);
   // light direction, view position, vertex position, and normal in tangent space
   vs_out.LightDir_tangent = TBN * lightDirection;
   vs_out.CamPos_tangent = TBN * viewPosition;
   vs_out.Pos_tangent  = vec3(0,0,0); // the position of the vertex in the tangent space is the origin of the space (we could ommit that)
   vs_out.Norm_tangent = TBN * N;

   // final vertex transform (for opengl rendering)
   gl_Position = projection * view * model * vec4(vertex, 1.0);
}