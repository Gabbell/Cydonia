#if CYD_PROFILING || CYD_GPU_PROFILING
#include <Profiling.h>
#endif

#if CYD_PROFILING
namespace CYD::Trace
{
void Initialize() { TracyCSetThreadName( "Main Thread" ); }
void FrameStart() { TracyCFrameMark; }
void FrameEnd() {}

static uint32_t GetU32ColorFromName( const char* name )
{
   std::hash<std::string> hasher;
   return static_cast<uint32_t>( hasher( std::string( name ) ) );
}

CPUScoped::CPUScoped(
    TracyCZoneCtx ctx,
    const std::string& text,
    const std::source_location& location )
    : m_ctx( ctx )
{
   const char* functionName = location.function_name();
   TracyCZoneName( m_ctx, functionName, strlen( functionName ) );
   TracyCZoneText( m_ctx, text.c_str(), strlen( text.c_str() ) );
   TracyCZoneColor( m_ctx, GetU32ColorFromName( functionName ) );
}

CPUScoped::~CPUScoped() { TracyCZoneEnd( m_ctx ); }
}
#else
namespace CYD::Trace
{
void Initialize() {}
void FrameStart() {}
void FrameEnd() {}
}
#endif

#if CYD_GPU_PROFILING
namespace CYD::Trace
{
std::array<float, 4> GetFloat4ColorFromName( const char* name )
{
   std::hash<std::string> hasher;
   const uint32_t hash = static_cast<uint32_t>( hasher( std::string( name ) ) );

   return {
       ( ( hash & 0x00FF0000 ) >> 16 ) / 255.0f,
       ( ( hash & 0x0000FF00 ) >> 8 ) / 255.0f,
       ( ( hash & 0x000000FF ) >> 0 ) / 255.0f,
       1.0f };
}

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
#endif