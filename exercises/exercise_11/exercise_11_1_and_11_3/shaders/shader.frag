#version 330 core
out vec4 FragColor;

in VS_OUT {
   vec3 normal;
   vec3 position;
} fin;

uniform vec3 cameraPos;
uniform samplerCube skybox;

uniform float reflectionFactor;
uniform float n2;
const float n1 = 1.0;

void main()
{
    vec3 I = fin.position - cameraPos; // incidence vector
    vec3 V = -normalize(I); // view vector (surface to camera)
    vec3 N = normalize(fin.normal); // surface normal

    // TODO exercise 10.1 - reflect camera to fragment vector and sample the skybox with the reflected direction
    vec3 reflectDir = reflect(I, fin.normal); // need to not be normalized, because otherwise the model would seem always at distance 1 from the eye
    vec4 reflectColor = texture(skybox, reflectDir);

    // TODO exercise 10.2 - refract the camera to fragment vector and sample the skybox with the reffracted direction
    vec3 refractDir = refract(I, fin.normal, n1/n2); // same as above
    vec4 refractColor = texture(skybox, refractDir);

    // TODO exercise 10.3 - implement the Schlick approximation of the Fresnel factor and set "reflectionProportion" accordingly
    float R0 = ((n1-n2)/(n1+n2)) * ((n1-n2)/(n1+n2));
    float viewAngleCos = dot(V,N); // need to be normalized, otherwise it wouldn't be the pur cosine of angle
    float reflectionProportion = R0 + (1.0 - R0) * pow((1.0 - viewAngleCos), 5);

    // we combine reflected and refracted color here
    FragColor = reflectionProportion * reflectColor + (1.0 - reflectionProportion) * refractColor;
}