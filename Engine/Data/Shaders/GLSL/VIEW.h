// VIEW.h
// Used in many vertex and fragment shaders

#define MAX_VIEWS 8

struct View
{
   vec4 pos;
   mat4 view;
   mat4 proj;
};

struct InverseView
{
   mat4 invView;
   mat4 invProj;
};