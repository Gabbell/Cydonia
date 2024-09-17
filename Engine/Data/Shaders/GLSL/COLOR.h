// COLOR.h
// Used for color related functions

vec3 GammaToLinear( vec3 color ) { return vec3( pow( color, vec3( 2.2 ) ) ); }
vec3 LinearToGamma( vec3 color ) { return vec3( pow( color, vec3( 1.0 / 2.2 ) ) ); }

vec3 ColorCorrect( vec3 luminance, float exposure )
{
   return vec3( 1.0 ) - exp( -luminance.rgb * exposure );
}