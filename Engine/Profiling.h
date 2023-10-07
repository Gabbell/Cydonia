#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <ThirdParty/Tracy/tracy/TracyC.h>

#include <array>
#include <string>

namespace CYD::Trace
{
#if CYD_PROFILING
#define CYD_TRACE( name )   \
   TracyCZone( ctx, true ); \
   const CYD::Trace::CPUScoped cpuTrace( ctx, name )
#else
#define CYD_TRACE( name )

#endif

#if CYD_GPU_PROFILING
#define CYD_SCOPED_GPUTRACE( cmdList, name ) const CYD::Trace::GPUScoped gpuTrace( cmdList, name )
#define CYD_GPUTRACE_BEGIN( cmdList, name ) \
   GRIS::BeginDebugRange( cmdList, name, CYD::Trace::GetFloat4ColorFromName( name ) );
#define CYD_GPUTRACE_END( cmdList ) GRIS::EndDebugRange( cmdList );
#else
#define CYD_SCOPED_GPUTRACE( cmdList, name )
#define CYD_GPUTRACE_BEGIN( cmdList, name )
#define CYD_GPUTRACE_END( cmdList )
#endif

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

void FrameStart();
void FrameEnd();

class CPUScoped final
{
  public:
   CPUScoped() = delete;
   CPUScoped( TracyCZoneCtx ctx, const char* name );
   MOVABLE( CPUScoped );
   ~CPUScoped();

  private:
   TracyCZoneCtx m_ctx;
};

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

}