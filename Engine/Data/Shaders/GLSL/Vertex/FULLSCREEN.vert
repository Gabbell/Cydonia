#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) out vec2 outUV;

void main()
{
   // Silly way to do CCW instead of CW because of culling
   uint vertexIndex = ( gl_VertexIndex + gl_VertexIndex ) % 3;

   outUV       = vec2( ( vertexIndex << 1 ) & 2, vertexIndex & 2 );
   gl_Position = vec4( outUV * 2.0f - 1.0f, 0.0f, 1.0f );
}