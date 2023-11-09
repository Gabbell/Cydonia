#pragma once

// ================================================================================================
// Stubs
namespace CYD::Trace
{
#if !CYD_PROFILING
#define CYD_TRACE()
#define CYD_TRACE_S( text ) ;

void Initialize();

void FrameStart();
void FrameEnd();
#endif

#if !CYD_GPU_PROFILING
#define CYD_SCOPED_GPUTRACE( cmdList, name )
#define CYD_GPUTRACE_BEGIN( cmdList, name )
#define CYD_GPUTRACE_END( cmdList )
#endif
}

// ================================================================================================
// Includes
#if CYD_PROFILING || CYD_GPU_PROFILING  // Common includes
#include <Common/Include.h>
#include <string>
#endif

#if CYD_PROFILING
#include <ThirdParty/Tracy/tracy/TracyC.h>
#include <source_location>
#endif

#if CYD_GPU_PROFILING
#include <Graphics/GRIS/RenderInterface.h>
#include <array>
#endif

// ================================================================================================
// CPU Profiling
#if CYD_PROFILING
namespace CYD::Trace
{
#define CYD_TRACE()         \
   TracyCZone( ctx, true ); \
   const CYD::Trace::CPUScoped cpuTrace( ctx )
#define CYD_TRACE_S( text ) \
   TracyCZone( ctx, true ); \
   const CYD::Trace::CPUScoped cpuTrace( ctx, text )

void Initialize();

void FrameStart();
void FrameEnd();

class CPUScoped final
{
  public:
   CPUScoped() = delete;
   CPUScoped(
       TracyCZoneCtx ctx,
       const std::string& text              = "",
       const std::source_location& location = std::source_location::current() );
   MOVABLE( CPUScoped );
   ~CPUScoped();

  private:
   TracyCZoneCtx m_ctx;
};
}
#endif

// ================================================================================================
// GPU Profiling
#if CYD_GPU_PROFILING
#define CYD_SCOPED_GPUTRACE( cmdList, name ) const CYD::Trace::GPUScoped gpuTrace( cmdList, name )
#define CYD_GPUTRACE_BEGIN( cmdList, name ) \
   GRIS::BeginDebugRange( cmdList, name, CYD::Trace::GetFloat4ColorFromName( name ) );
#define CYD_GPUTRACE_END( cmdList ) GRIS::EndDebugRange( cmdList );

namespace CYD::Trace
{
class GPUScoped final
{
  public:
   GPUScoped() = delete;
   GPUScoped( CmdListHandle cmdList, const char* name );
   MOVABLE( GPUScoped );
   ~GPUScoped();

  private:
   CmdListHandle m_cmdList;
};

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
}
#endif