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

// from http://www.java-gaming.org/index.php?topic=35123.0
vec4 cubic( float v )
{
   vec4 n  = vec4( 1.0, 2.0, 3.0, 4.0 ) - v;
   vec4 s  = n * n * n;
   float x = s.x;
   float y = s.y - 4.0 * s.x;
   float z = s.z - 4.0 * s.y + 6.0 * s.x;
   float w = 6.0 - x - y - z;
   return vec4( x, y, z, w ) * ( 1.0 / 6.0 );
}

vec4 textureBicubic( sampler2D tex, vec2 uv )
{
   vec2 texSize    = textureSize( tex, 0 );
   vec2 invTexSize = 1.0 / texSize;

   uv = uv * texSize - 0.5;

   vec2 fxy = fract( uv );
   uv -= fxy;

   vec4 xcubic = cubic( fxy.x );
   vec4 ycubic = cubic( fxy.y );

   vec4 c = uv.xxyy + vec2( -0.5, +1.5 ).xyxy;

   vec4 s      = vec4( xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw );
   vec4 offset = c + vec4( xcubic.yw, ycubic.yw ) / s;

   offset *= invTexSize.xxyy;

   vec4 sample0 = texture( tex, offset.xz );
   vec4 sample1 = texture( tex, offset.yz );
   vec4 sample2 = texture( tex, offset.xw );
   vec4 sample3 = texture( tex, offset.yw );

   float sx = s.x / ( s.x + s.y );
   float sy = s.z / ( s.z + s.w );

   return mix( mix( sample3, sample2, sx ), mix( sample1, sample0, sx ), sy );
}