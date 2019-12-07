layout(binding = 0, std140) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} _19;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec4 inColor;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) in vec2 inTexCoord;

void main()
{
    gl_Position = ((_19.proj * _19.view) * _19.model) * vec4(inPosition, 1.0);
    outColor = inColor;
    outTexCoord = inTexCoord;
}

