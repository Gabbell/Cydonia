layout(binding = 0, std140) uniform UBO
{
    mat4 mv;
    mat4 proj;
}ubo;

layout(location = 0) in vec4 inPosition;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec4 inColor;

void main()
{
    gl_Position = ubo.proj * ubo.mv * inPosition;
    outColor = inColor;
}

