layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 0) in vec4 inColor;

void main()
{
    outColor = texture(texSampler, inTexCoord);
}

