// NOISE.h
// Used in noise compute shaders

layout( push_constant ) uniform NoiseParameters
{
   float width;
   float height;
   float seed;
   float frequency;
   float scale;
   float exponent;
   bool normalize;
   bool absolute;
   uint octaves;
};

float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio

vec3 mod289( vec3 x ) { return x - floor( x * ( 1.0 / 289.0 ) ) * 289.0; }
vec2 mod289( vec2 x ) { return x - floor( x * ( 1.0 / 289.0 ) ) * 289.0; }
vec3 permute( vec3 x ) { return mod289( ( ( x * 34.0 ) + 1.0 ) * x ); }

float rand( vec2 p )
{
   vec3 p3 = fract( vec3( p.xyx ) * .1031 );
   p3 += dot( p3, p3.yzx + 33.33 );
   return fract( ( p3.x + p3.y ) * p3.z );
}
