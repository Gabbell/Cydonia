struct AtmosphereParameters
{
   vec4 mieScatteringCoefficient;       // XYZ = Coeff, W = Scale
   vec4 mieAbsorptionCoefficient;       // XYZ = Coeff, W = Scale
   vec4 rayleighScatteringCoefficient;  // XYZ = Coeff, W = Scale
   vec4 absorptionCoefficient;          // XYZ = Coeff, W = Scale
   vec4 groundAlbedo;
   vec4 heightFog;  // X = Height, Y = Falloff, Z = Strength, W = Unused
   float groundRadiusMM;
   float atmosphereRadiusMM;
   float miePhase;
   float rayleighHeight;
   float mieHeight;
   float nearClip;
   float farClip;
   float time;
};

const float PI      = 3.14159265358;
const float EPSILON = 0.00001;

const vec3 ozoneAbsorptionBase = vec3( 0.650, 1.881, 0.085 );

// Hard-coded orthogonal basis
const vec3 right   = vec3( 1.0, 0.0, 0.0 );
const vec3 up      = vec3( 0.0, 1.0, 0.0 );
const vec3 forward = vec3( 0.0, 0.0, 1.0 );

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

vec3 getMMPosition( float groundRadiusMM, vec3 viewPos )
{
   return vec3( viewPos / 1e6 ) + vec3( 0.0, groundRadiusMM, 0.0 );
}

vec2 LUTParameterization( float groundRadiusMM, float atmosphereRadiusMM, vec3 sunDir, vec3 posMM )
{
   const float height = posMM.y;
   const vec3 up      = posMM / height;  // Effectively normalizing, but we need the height later

   const float sunCosZenithAngle = dot( sunDir, up );
   return vec2(
       0.5 + 0.5 * sunCosZenithAngle,
       clamp( ( height - groundRadiusMM ) / ( atmosphereRadiusMM - groundRadiusMM ), 0.0, 1.0 ) );
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
    out vec3 mieScattering,
    out vec3 extinction )
{
   const float altitudeKM = ( pos.y - params.groundRadiusMM ) * 1e3;

   // Note: Paper gets these switched up.
   const float rayleighDensity = exp( -altitudeKM / params.rayleighHeight );
   const float mieDensity      = exp( -altitudeKM / params.mieHeight );

   rayleighScattering = 1e3 * params.rayleighScatteringCoefficient.a *
                        params.rayleighScatteringCoefficient.rgb * rayleighDensity;

   const vec3 rayleighAbsorption = vec3( 0.0 );

   mieScattering =
       1e3 * params.mieScatteringCoefficient.a * params.mieScatteringCoefficient.rgb * mieDensity;

   const vec3 mieAbsorption =
       1e3 * params.mieAbsorptionCoefficient.a * params.mieAbsorptionCoefficient.rgb * mieDensity;

   const vec3 ozoneAbsorption = 1e3 * params.absorptionCoefficient.a *
                                params.absorptionCoefficient.rgb *
                                max( 0.0, 1.0 - abs( altitudeKM - 25.0 ) / 15.0 );

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
float getMiePhase( float miePhase, float cosTheta )
{
   const float g     = miePhase;
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
   return ( depth * maxSlice ) - 1.0;
}

// ===============================================================================================
// ARCHIVED
vec3 GetSunFog( AtmosphereParameters params, vec3 rayDir, vec3 sunDir, float height )
{
   const float cosTheta = dot( rayDir, sunDir );

   const float diff = 1.0 - cosTheta;

   // The higher we are in the atmosphere, the less sun fog we have
   // This probably would not be needed with the aerial perspective LUT
   const float heightRatio =
       ( height - params.groundRadiusMM ) / ( params.atmosphereRadiusMM - params.groundRadiusMM );

   const float heighFactor = exp( -5.0 * heightRatio );

   // Sun fog
   return heighFactor * vec3( exp( -diff * 100.0 ) );
}