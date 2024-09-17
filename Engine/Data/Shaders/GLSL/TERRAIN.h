// TERRAIN.h
// Functions and constants used to render terrain

const uint TERRAIN_TILING_X = 16;
const uint TERRAIN_TILING_Y = 16;

float GetDisplacement( sampler2DArray heightMap, vec3 uv )
{
   float displacement = texture( heightMap, uv ).r;
   if( uv.xy == vec2( 0.0, 0.0 ) )
   {
      // Top left
      displacement += texture( heightMap, vec3( 1.0, 0.0, uv.z - 1 ) ).r;
      displacement += texture( heightMap, vec3( 0.0, 1.0, uv.z - TERRAIN_TILING_X ) ).r;
      displacement += texture( heightMap, vec3( 1.0, 1.0, uv.z - 1 - TERRAIN_TILING_X ) ).r;
      displacement /= 4.0;
   }
   else if( uv.xy == vec2( 1.0, 0.0 ) )
   {
      // Top right
      displacement += texture( heightMap, vec3( 0.0, 0.0, uv.z + 1 ) ).r;
      displacement += texture( heightMap, vec3( 1.0, 1.0, uv.z - TERRAIN_TILING_X ) ).r;
      displacement += texture( heightMap, vec3( 0.0, 1.0, uv.z + 1 - TERRAIN_TILING_X ) ).r;
      displacement /= 4.0;
   }
   else if( uv.xy == vec2( 0.0, 1.0 ) )
   {
      // Bottom left
      displacement += texture( heightMap, vec3( 1.0, 1.0, uv.z - 1 ) ).r;
      displacement += texture( heightMap, vec3( 0.0, 0.0, uv.z + TERRAIN_TILING_X ) ).r;
      displacement += texture( heightMap, vec3( 1.0, 0.0, uv.z - 1 + TERRAIN_TILING_X ) ).r;
      displacement /= 4.0;
   }
   else if( uv.xy == vec2( 1.0, 1.0 ) )
   {
      // Bottom right
      displacement += texture( heightMap, vec3( 0.0, 1.0, uv.z + 1 ) ).r;
      displacement += texture( heightMap, vec3( 1.0, 0.0, uv.z + TERRAIN_TILING_X ) ).r;
      displacement += texture( heightMap, vec3( 0.0, 0.0, uv.z + 1 + TERRAIN_TILING_X ) ).r;
      displacement /= 4.0;
   }
   else if( uv.x == 0.0 )
   {
      // Left edge
      displacement += texture( heightMap, vec3( 1.0, uv.y, uv.z - 1 ) ).r;
      displacement /= 2.0;
   }
   else if( uv.x == 1.0 )
   {
      // Right edge
      displacement += texture( heightMap, vec3( 0.0, uv.y, uv.z + 1 ) ).r;
      displacement /= 2.0;
   }
   else if( uv.y == 0.0 )
   {
      // Top edge
      displacement += texture( heightMap, vec3( uv.x, 1.0, uv.z - TERRAIN_TILING_X ) ).r;
      displacement /= 2.0;
   }
   else if( uv.y == 1.0 )
   {
      // Bottom edge
      displacement += texture( heightMap, vec3( uv.x, 0.0, uv.z + TERRAIN_TILING_X ) ).r;
      displacement /= 2.0;
   }

   return displacement;
}

// ================================================================================================