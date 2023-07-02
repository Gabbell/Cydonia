// INSTANCING.h
// Used for instancing in vertex shaders

// ================================================================================================
// Keep these defines and structs in sync with "InstanceShaderParams"
#define MAX_INSTANCES 1024

struct InstanceData
{
   mat4x4 modelMat;
};

layout( set = 0, binding = 1 ) uniform InstancesData { InstanceData instances[MAX_INSTANCES]; };
// ================================================================================================