#include <Profiling.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <string>

namespace CYD::Trace
{
void FrameStart()
{
#if CYD_PROFILING
   TracyCFrameMark;
#endif
}

void FrameEnd()
{
#if CYD_PROFILING
#endif
}

static uint32_t GetU32ColorFromName( const char* name )
{
   std::hash<std::string> hasher;
   return static_cast<uint32_t>( hasher( std::string( name ) ) );
}

static const std::array<float, 4> GetFloat4ColorFromName( const char* name )
{
   std::hash<std::string> hasher;
   const uint32_t hash = static_cast<uint32_t>( hasher( std::string( name ) ) );

   return {
       ( ( hash & 0x00FF0000 ) >> 16 ) / 255.0f,
       ( ( hash & 0x0000FF00 ) >> 8 ) / 255.0f,
       ( ( hash & 0x000000FF ) >> 0 ) / 255.0f,
       1.0f };
}

CPUScoped::CPUScoped( TracyCZoneCtx ctx, const char* name ) : m_ctx( ctx )
{
   TracyCZoneName( m_ctx, name, strlen( name ) );
   TracyCZoneColor( m_ctx, GetU32ColorFromName( name ) );
}

CPUScoped::~CPUScoped() { TracyCZoneEnd( m_ctx ); }

GPUScoped::GPUScoped( CmdListHandle cmdList, const char* name ) : m_cmdList( cmdList )
{
   if( m_cmdList )
   {
      GRIS::BeginDebugRange( m_cmdList, name, GetFloat4ColorFromName( name ) );
   }
}

GPUScoped::~GPUScoped()
{
   if( m_cmdList )
   {
      GRIS::EndDebugRange( m_cmdList );
   }
}
}