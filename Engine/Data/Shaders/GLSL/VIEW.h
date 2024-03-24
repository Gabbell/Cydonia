// VIEW.h
// Used in many vertex and fragment shaders

#define MAX_VIEWS 8

struct View
{
   vec4 pos;
   mat4 viewMat;
   mat4 projMat;
};

struct InverseView
{
   vec4 pos;
   mat4 invViewMat;
   mat4 invProjMat;
};

struct Frustum
{
   vec4 planes[6];
};