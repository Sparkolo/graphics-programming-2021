#version 330 core
layout (location = 0) in vec2 pos;   // the position variable has attribute position 0
layout (location = 1) in vec2 vel;   // the velocity variable has attribute position 1
layout (location = 2) in float startTime;    // the time of birth variable has attribute position 2

uniform float currentTime;

out float age;

void main()
{
    age = currentTime - startTime;

    vec2 finalPos = pos;
    float maxAge = 10.0f;
    float minSize = 0.1f;
    float maxSize = 200.0f;

    if(startTime == .0f)
        finalPos.x = 2.0f;
    else {
        if (age < maxAge)
            finalPos += age * vel - age * vec2(0.0f, 0.0981f);
        else
            finalPos.x = 2.0f;
    }

    gl_Position = vec4(finalPos, 0.0, 1.0);
    gl_PointSize = mix(minSize, maxSize, pow(age / maxAge, 2));
}