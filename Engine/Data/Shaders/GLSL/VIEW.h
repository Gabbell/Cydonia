// VIEW.h
// Used in many vertex and fragment shaders

#define MAX_VIEWS 8

struct View
{
   vec4 pos;
   mat4 view;
   mat4 proj;
};

layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };

float GetFarPlane(mat4 proj)
{
	// This is for a perspective reverse Z projection matrix
	// We're effectively getting the near plane instead
	return proj[3][2] / proj[2][2];
}