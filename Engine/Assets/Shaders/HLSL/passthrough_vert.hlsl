layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec4 inColor;

void main()
{
    gl_Position = vec4(inPosition, 1.0);
    outColor = inColor;
}

