struct TessellationParams
{
   vec2 viewportDims;
   float tessellatedEdgeSize;
   float tessellationFactor;
};

#define TESS_TERRAIN_MIN_DISTANCE 10
#define TESS_TERRAIN_MAX_DISTANCE 8000
#define TESS_TERRAIN_MIN_LEVEL 1
#define TESS_TERRAIN_MAX_LEVEL 64

// Tessellation function based on screen-space size
float ScreenSpaceTessFactor(
    TessellationParams params,
    mat4 instanceModelMat,
    mat4 viewMat,
    mat4 projMat,
    vec4 p0,
    vec4 p1 )
{
   // Calculate edge mid point
   vec4 midPoint = 0.5 * ( p0 + p1 );

   // Sphere radius as distance between the control points
   float radius = distance( p0, p1 ) / 2.0;

   // View space
   vec4 v0 = viewMat * instanceModelMat * midPoint;

   // Project into clip space
   vec4 clip0 = ( projMat * ( v0 - vec4( radius, vec3( 0.0 ) ) ) );
   vec4 clip1 = ( projMat * ( v0 + vec4( radius, vec3( 0.0 ) ) ) );

   // Get normalized device coordinates
   clip0 /= clip0.w;
   clip1 /= clip1.w;

   // Convert to viewport coordinates
   clip0.xy *= params.viewportDims;
   clip1.xy *= params.viewportDims;

   // Return the tessellation factor based on the screen size
   // given by the distance of the two edge control points in screen space
   // and a reference (min.) tessellation size for the edge set by the application
   return clamp(
       distance( clip0, clip1 ) / params.tessellatedEdgeSize * params.tessellationFactor,
       1.0,
       64.0 );  // 64 is the minimum maximum of tessellation levels
}
