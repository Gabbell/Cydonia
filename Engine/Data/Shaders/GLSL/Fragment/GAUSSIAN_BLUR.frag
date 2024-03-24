#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( push_constant ) uniform PUSH { bool isHorizontal; };

layout( set = 0, binding = 0 ) uniform sampler2D color;

layout( location = 0 ) in vec2 inUV;

layout( location = 0 ) out vec4 outColor;

const uint KERNEL_SIZE = 5;

const float gaussian_kernel[KERNEL_SIZE] = float[KERNEL_SIZE]
(	0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216   );

// =================================================================================================
void main()
{
  const vec2 texelSize = 1.0 / textureSize(color, 0);
  const vec2 direction = isHorizontal ? vec2( texelSize.x, 0.0 ) : vec2( 0.0, texelSize.y );
  vec4 sum = gaussian_kernel[0] * texture( color, inUV );

  for ( uint i = 1; i < KERNEL_SIZE; i++ )
  {
    sum += gaussian_kernel[i] * texture( color, inUV + direction * float(i) );
    sum += gaussian_kernel[i] * texture( color, inUV - direction * float(i) );
  }

  outColor = sum;
}
