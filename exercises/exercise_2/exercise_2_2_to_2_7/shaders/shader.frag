#version 330 core
in vec2 gl_PointCoord;
in float age;

out vec4 fragColor;

void main()
{
    float xDist = gl_PointCoord.x - 0.5f;
    float yDist = gl_PointCoord.y - 0.5f;
    float dist = sqrt(xDist * xDist + yDist * yDist);
    dist = dist > 0.5f ? 0.5f : dist;

    float maxAge = 10.0f;
    float agePercentage = age/maxAge;

    vec3 finalColor;
    if(agePercentage < 0.5f)
        finalColor = vec3(1.0f,
                          mix(1.0f, 0.5f, agePercentage * 2),
                          mix(0.05f, 0.01f, agePercentage * 2));
    else
        finalColor = vec3(mix(1.0f, 0.0f, (agePercentage - 0.5f) * 2),
                          mix(0.5f, 0.0f, (agePercentage - 0.5f) * 2),
                          mix(0.01f, 0.0f, (agePercentage - 0.5f) * 2));

    fragColor = vec4(finalColor, mix(1.0f - dist * 2.0, 0.0f, agePercentage));
}