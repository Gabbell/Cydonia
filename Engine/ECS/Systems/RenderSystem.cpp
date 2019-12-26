#include <ECS/Systems/RenderSystem.h>

bool cyd::RenderSystem::init()
{
   data = new unsigned char[512];
   return true;
}

void cyd::RenderSystem::tick( double deltaMs ) { printf( "RenderSystem ticking\n" ); }

cyd::RenderSystem::~RenderSystem() { delete[] data; }
