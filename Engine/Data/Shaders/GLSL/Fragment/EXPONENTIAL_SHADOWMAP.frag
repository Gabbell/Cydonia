#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../SHADOW.h"

layout( location = 0 ) out vec4 outColor;

void main()
{
	const float linearDepth = clamp( gl_FragCoord.z, 0.0, 1.0 );
	outColor = vec4( exp( EXP_CONSTANT * gl_FragCoord.z ), 0.0, 0.0, 1.0 );
}