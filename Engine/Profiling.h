#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <ThirdParty/Tracy/tracy/TracyC.h>

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
#define CYD_GPUTRACE( cmdList, name ) const CYD::Trace::GPUScoped gpuTrace( cmdList, name )
#else
#define CYD_GPUTRACE( cmdList, name )
#endif

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