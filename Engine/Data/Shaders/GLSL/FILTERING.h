// FILTERING.h
// Used in texture sampling

vec4 textureBilinear( sampler2D tex, vec2 P )
{
   ivec2 textureSize = textureSize( tex, 0 );
   vec2 texelSize    = 1.0 / textureSize;

   vec2 pixel = P * textureSize + 0.5;

   vec2 frac = fract( pixel );
   pixel     = ( floor( pixel ) / textureSize ) - vec2( texelSize / 2.0 );

   vec4 C11 = texture( tex, pixel + vec2( 0.0, 0.0 ) );
   vec4 C21 = texture( tex, pixel + vec2( texelSize.x, 0.0 ) );
   vec4 C12 = texture( tex, pixel + vec2( 0.0, texelSize.y ) );
   vec4 C22 = texture( tex, pixel + vec2( texelSize.x, texelSize.y ) );

   vec4 x1 = mix( C11, C21, frac.x );
   vec4 x2 = mix( C12, C22, frac.x );
   return mix( x1, x2, frac.y );
}