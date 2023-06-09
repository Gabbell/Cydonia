#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <ThirdParty/Tracy/tracy/TracyC.h>

namespace CYD::Trace
{
#if CYD_PROFILING
#define CYDTRACE( name )    \
   TracyCZone( ctx, true ); \
   const CYD::Trace::CPUScoped cpuTrace( ctx, name )

#define CYDGPUTRACE( cmdList, name ) const CYD::Trace::GPUScoped gpuTrace( cmdList, name )
#else
#define CYDTRACE( name )
#define CYDGPUTRACE( cmdList, name )
#endif

void FrameStart();
void FrameEnd();

class CPUScoped
{
  public:
   CPUScoped() = delete;
   CPUScoped( TracyCZoneCtx ctx, const char* name );
   MOVABLE( CPUScoped );
   ~CPUScoped();

  private:
   TracyCZoneCtx m_ctx;
};

class GPUScoped
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