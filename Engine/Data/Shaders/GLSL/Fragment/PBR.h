// PBR.h
// Used for physically based lighting

const float PI = 3.141592653589793;

// DIFFUSE
// ================================================================================================

// SPECULAR
// ================================================================================================

// GGX/Trowbridge-Reitz Normal Distribution Function (NDF)
// Uses Disney's reparameterization of alpha = roughness^2
// A statistical approximation of how many microfacet surfaces are perpendicular to
// the halfway vector, i.e how many surface normals align with the halfway vector
// This is approximated using the roughness map and affects specular lighting
float DistributionGGX( float NdotH, float roughness )
{
   float alpha   = roughness * roughness;
   float alphaSq = alpha * alpha;

   float denom = ( NdotH * NdotH * ( alphaSq - 1.0 ) + 1.0 );
   denom       = PI * denom * denom;

   return alphaSq / denom;
}

// Schlick-GGX term
// Combination of GGX and Schlick-Beckmann aproximation
float GeometrySchlickGGX( float cosTheta, float k )
{
   float nom   = cosTheta;
   float denom = cosTheta * ( 1.0 - k ) + k;

   return nom / denom;
}

// Geometrical Attenuation Function using Smith's method
// Based on "Geometrical shadowing of a random rough surface" by B. Smith
// A statistical approximation of the self-shadowing properties of the microfacets
// on the surface The rougher a surface is, the more surface diffusion and
// self-shadowing occurs and therefore reducing the light the surface reflects.
// This is determined by roughness and affects the diffuse gradient
float GeometrySmith( float NdotL, float NdotV, float roughness )
{
   float r = ( roughness + 1.0 );
   float k = ( r * r ) / 8.0;

   return GeometrySchlickGGX( NdotL, k ) * GeometrySchlickGGX( NdotV, k );
}

// Fresnel-Shlick approximation of the Fresnel term
// The Fresnel term describes the proportion of light getting reflected against
// the light that gets refracted which depends on the incident angle
vec3 FresnelSchlick( float cosTheta, vec3 F0 )
{
   return F0 + ( 1.0 - F0 ) * pow( clamp( 1.0 - cosTheta, 0.0, 1.0 ), 5.0 );
}

// Cook-Torrance BRDF
vec3 ComputeLightCookTorranceBRDF(
    vec3 lightRadiance,
    vec3 lightDir,
    vec3 viewDir,
    vec3 albedo,  // Assuming linear values please and thank you
    vec3 normal,  // Assuming normal in world space
    float metalness,
    float roughness,
    float ao )
{
   const vec3 V = viewDir;                // View direction
   const vec3 L = normalize( lightDir );  // Light direction
   const vec3 N = normalize( normal );    // Normal
   const vec3 H = normalize( V + L );     // Halfway light vector

   float NdotV = clamp( dot( N, V ), 0.0, 1.0 );
   float NdotL = clamp( dot( N, L ), 0.0, 1.0 );
   float NdotH = clamp( dot( N, H ), 0.0, 1.0 );
   float HdotV = clamp( dot( H, V ), 0.0, 1.0 );

   // Calculate Specular
   vec3 specularColor = vec3( 0.04 );
   specularColor      = mix( specularColor, albedo, metalness );

   const float NDF   = DistributionGGX( NdotH, roughness );
   const float G     = GeometrySmith( NdotL, NdotV, roughness );
   const vec3 F      = FresnelSchlick( HdotV, specularColor );
   vec3 specularTerm = ( NDF * G * F ) / ( 4.0 * NdotV * NdotL + 0.001 );

   // Calculate Diffuse based on the energy conservation principle
   // The diffuse radiance must be the inverse of the specular radiance so that we
   // don't create energy
   vec3 kD = vec3( 1.0, 1.0, 1.0 ) - F;
   kD *= 1.0 - metalness;
   vec3 diffuseTerm = kD * albedo;

   // Returns linear value
   return ( diffuseTerm + specularTerm ) * lightRadiance * NdotL;
}