struct AtmosphereParameters
{
   float groundRadiusMM;
   float atmosphereRadiusMM;
   float phaseScale;
   float nearClip;
   float farClip;
   float heightFogA;
   float heightFogB;
   float time;
};

const float PI      = 3.14159265358;
const float EPSILON = 0.00001;

const float rayleighAbsorptionBase = 0.0;
const vec3 rayleighScatteringBase  = vec3( 5.802, 13.558, 33.1 );

const float mieScatteringBase = 3.996;
const float mieAbsorptionBase = 4.4;

const vec3 ozoneAbsorptionBase = vec3( 0.650, 1.881, 0.085 );

// The albedo bouncing off the planet
const vec3 groundAlbedo = vec3( 0.3 );

// Hard-coded up orthogonal basis
const vec3 up      = vec3( 0.0, 1.0, 0.0 );
const vec3 forward = vec3( 0.0, 0.0, -1.0 );
const vec3 right   = vec3( 1.0, 0.0, 0.0 );

// Hard-coded planet position
const vec3 spherePos = vec3( 0.0, 0.0, 0.0 );

// From https://gamedev.stackexchange.com/questions/96459/fast-ray-sphere-collision-code
float rayIntersectSphere( vec3 ro, vec3 rd, vec3 so, float radius )
{
   vec3 m  = ro - so;
   float b = dot( m, rd );
   float c = dot( m, m ) - ( radius * radius );

   // Exit if ro is outside sphere (c > 0) and rd pointing away from sphere (b > 0)
   if( c > 0.0 && b > 0.0 ) return -1.0;

   float discr = b * b - c;
   if( discr < 0.0 ) return -1.0;

   // Special case: inside sphere, use far discriminant
   if( discr > b * b ) return ( -b + sqrt( discr ) );
   return -b - sqrt( discr );
}

vec3 getDirFromSpherical( float theta, float phi )
{
   // (r, theta, phi) - (radial distance, zenith angle, azimuth angle )
   // following the convention in the paper
   const float cosTheta = cos( theta );
   const float sinTheta = sin( theta );
   const float cosPhi   = cos( phi );
   const float sinPhi   = sin( phi );
   return vec3( sinTheta * sinPhi, cosPhi, sinPhi * cosTheta );
}

vec3 getMMPosition( AtmosphereParameters params, vec3 viewPos )
{
   return vec3( viewPos / 1e6 ) + vec3(0.0, params.groundRadiusMM, 0.0 );
}

vec2 LUTParameterization( AtmosphereParameters params, vec3 sunDir, vec3 pos )
{
   const vec3 mmPosition = getMMPosition( params, pos );
   const float height    = length( pos );
   const vec3 up = mmPosition / height;  // Effectively normalizing, but we need the height later

   const float sunCosZenithAngle = dot( sunDir, up );
   return vec2(
       0.5 + 0.5 * sunCosZenithAngle,
       clamp(
           ( height - params.groundRadiusMM ) /
               ( params.atmosphereRadiusMM - params.groundRadiusMM ),
           0.0,
           1.0 ) );
}

// XYZ = Sun Direction
// W = Height
vec4 inverseLUTParameterization( AtmosphereParameters params, vec2 uv )
{
   const float sunCosTheta = 2.0 * uv.x - 1.0;
   const float sunTheta    = acos( sunCosTheta );  // Zenith angle
   const float height      = mix( params.groundRadiusMM, params.atmosphereRadiusMM, uv.y );

   const vec3 sunDir = normalize( vec3( 0.0, sunCosTheta, -sin( sunTheta ) ) );

   return vec4( sunDir, height );
}

void getScatteringValues(
    vec3 pos,
    AtmosphereParameters params,
    out vec3 rayleighScattering,
    out float mieScattering,
    out vec3 extinction )
{
   const float altitudeKM = ( length( pos ) - params.groundRadiusMM ) * 1000.0;

   // Note: Paper gets these switched up.
   const float rayleighDensity = exp( -altitudeKM / 8.0 );
   const float mieDensity      = exp( -altitudeKM / 1.2 );

   rayleighScattering             = rayleighScatteringBase * rayleighDensity;
   const float rayleighAbsorption = rayleighAbsorptionBase * rayleighDensity;

   mieScattering             = mieScatteringBase * mieDensity;
   const float mieAbsorption = mieAbsorptionBase * mieDensity;

   const vec3 ozoneAbsorption =
       ozoneAbsorptionBase * max( 0.0, 1.0 - abs( altitudeKM - 25.0 ) / 15.0 );

   extinction =
       rayleighScattering + rayleighAbsorption + mieScattering + mieAbsorption + ozoneAbsorption;
}

// ===============================================================================================
// Phase functions
// Distribution of light direction after a scatter event

float getRayleighPhase( float cosTheta )
{
   return ( 3.0 * ( 1.0 + cosTheta * cosTheta ) ) / ( 16.0 * PI );
}

// Cornette-Shanks phase function
float getMiePhase( float cosTheta )
{
   const float g     = 0.8;
   const float scale = 3.0 / ( 8.0 * PI );

   const float num   = ( 1.0 - g * g ) * ( 1 + cosTheta * cosTheta );
   const float denom = ( 2.0 + g * g ) * pow( ( 1.0 + g * g - 2.0 * g * cosTheta ), 1.5 );

   return scale * num / denom;
}

// ===============================================================================================
// Aerial Perspective
// Note that maxDistanceMM must somewhat match with the far clip distance of the camera

float SliceToDistanceMM( float slice, float maxDistanceMM, float maxSlice )
{
   // First slice is not at the camera depth (0) but at (maxDistanceMM / maxSlice)
   return ( slice + 1.0 ) * ( maxDistanceMM / maxSlice );
}

float DepthToSlice( float depth, float maxSlice )
{
   // This returns between -1 and 0 for the part between the camera and the first slice
   return (depth * maxSlice) - 1.0;
}