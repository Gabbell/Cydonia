// RAYMARCH.h
// Used in raymarch compute shaders

// Constants
#define PI 3.1415925359
#define TWO_PI 6.2831852
#define MAX_STEPS 100
#define MAX_DIST 100.0
#define SURFACE_DIST 0.01

struct RaymarchParams
{
	uint width;
	uint height;
	uint octaves;
	float time;
};	
