// INSTANCING.h
// Used for instancing in vertex shaders

// ================================================================================================
// Keep these defines and structs in sync with "InstanceShaderParams"
#define MAX_INSTANCES 1024

struct InstancingData
{
   uint index;
   mat4x4 modelMat;
};

// ================================================================================================