// NOISE.h
// Used in noise compute shaders

layout( push_constant ) uniform NoiseParameters
{
   float seed;
   float amplitude;
   float gain;
   float frequency;
   float lacunarity;
   float exponent;
   bool ridged;
   bool invert;
   uint octaves;
}
params;

float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio

vec3 mod289( vec3 x ) { return x - floor( x * ( 1.0 / 289.0 ) ) * 289.0; }
vec2 mod289( vec2 x ) { return x - floor( x * ( 1.0 / 289.0 ) ) * 289.0; }
vec3 permute( vec3 x ) { return mod289( ( ( x * 34.0 ) + 1.0 ) * x ); }

float hash1( float n ) { return fract( sin( n ) * 43758.5453 ); }
vec2 hash2( vec2 p )
{
   p = vec2( dot( p, vec2( 127.1, 311.7 ) ), dot( p, vec2( 269.5, 183.3 ) ) );
   return fract( sin( p ) * 43758.5453 );
}

float rand( vec2 p )
{
   vec3 p3 = fract( vec3( p.xyx ) * .1031 );
   p3 += dot( p3, p3.yzx + 33.33 );
   return fract( ( p3.x + p3.y ) * p3.z );
}

// White Noise implementation using the golden ratio
// https://www.shadertoy.com/view/ltB3zD
float GoldNoise( in vec2 xy )
{
   return fract( tan( distance( xy * PHI, xy ) * params.seed ) * xy.x );
}

float NoiseFinalize( float noiseValue )
{
   // Apply exponent
   noiseValue = pow( noiseValue, params.exponent );

   // Sanitizing output
   if( isnan( noiseValue ) ) noiseValue = 0.0;  // If is nan, just clamp to 0.0
   if( isinf( noiseValue ) ) noiseValue = 1.0;

   // Clamping. Maybe this should be optional?
   noiseValue = clamp( noiseValue, 0.0, 1.0 );

   // Invert the values
   if( params.invert ) noiseValue = 1.0 - noiseValue;

   return noiseValue;
}

// Description : GLSL 2D simplex noise function
//      Author : Ian McEwan, Ashima Arts
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License :
//  Copyright (C) 2011 Ashima Arts. All rights reserved.
//  Distributed under the MIT License. See LICENSE file.
//  https://github.com/ashima/webgl-noise
//
float SimplexNoise( vec2 v )
{
   // Precompute values for skewed triangular grid
   const vec4 C = vec4(
       0.211324865405187,
       // (3.0-sqrt(3.0))/6.0
       0.366025403784439,
       // 0.5*(sqrt(3.0)-1.0)
       -0.577350269189626,
       // -1.0 + 2.0 * C.x
       0.024390243902439 );
   // 1.0 / 41.0

   // First corner (x0)
   vec2 i  = floor( v + dot( v, C.yy ) );
   vec2 x0 = v - i + dot( i, C.xx );

   // Other two corners (x1, x2)
   vec2 i1 = vec2( 0.0 );
   i1      = ( x0.x > x0.y ) ? vec2( 1.0, 0.0 ) : vec2( 0.0, 1.0 );
   vec2 x1 = x0.xy + C.xx - i1;
   vec2 x2 = x0.xy + C.zz;

   // Do some permutations to avoid
   // truncation effects in permutation
   i      = mod289( i );
   vec3 p = permute( permute( i.y + vec3( 0.0, i1.y, 1.0 ) ) + i.x + vec3( 0.0, i1.x, 1.0 ) );

   vec3 m = max( 0.5 - vec3( dot( x0, x0 ), dot( x1, x1 ), dot( x2, x2 ) ), 0.0 );

   m = m * m;
   m = m * m;

   // Gradients:
   //  41 pts uniformly over a line, mapped onto a diamond
   //  The ring size 17*17 = 289 is close to a multiple
   //      of 41 (41*7 = 287)

   vec3 x  = 2.0 * fract( p * C.www ) - 1.0;
   vec3 h  = abs( x ) - 0.5;
   vec3 ox = floor( x + 0.5 );
   vec3 a0 = x - ox;

   // Normalise gradients implicitly by scaling m
   // Approximation of: m *= inversesqrt(a0*a0 + h*h);
   m *= 1.79284291400159 - 0.85373472095314 * ( a0 * a0 + h * h );

   // Compute final noise value at P
   vec3 g = vec3( 0.0 );
   g.x    = a0.x * x0.x + h.x * x0.y;
   g.yz   = a0.yz * vec2( x1.x, x2.x ) + h.yz * vec2( x1.y, x2.y );
   return 130.0 * dot( m, g );
}

// Fractal Brownian Motion
float SimplexFBM( vec2 uv )
{
   float finalNoise = 0.0f;

   // Initial frequency and amplitude for first octave
   float frequency = params.frequency;
   float amplitude = params.amplitude;

   mat2 rot = mat2( cos( 0.5 ), sin( 0.5 ), -sin( 0.5 ), cos( 0.50 ) );
   for( uint i = 0; i < params.octaves; ++i )
   {
      float noiseValue = SimplexNoise( uv * frequency );
      if( params.ridged )
      {
         noiseValue = abs( noiseValue );
      }
      else
      {
         noiseValue = noiseValue * 0.5 + 0.5;
      }

      finalNoise += amplitude * noiseValue;

      amplitude *= params.gain;
      frequency *= params.lacunarity;

      uv = uv * rot;  // Rotate UV to reduce axial bias
   }

   return NoiseFinalize( finalNoise );
}