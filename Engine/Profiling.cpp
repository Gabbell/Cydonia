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