#pragma once

#include <Common/Include.h>

#include <Graph/NodeGraph.h>

#include <Graphics/Handles/ResourceHandle.h>

namespace CYD::RenderGraph
{
enum Pass
{
   LOAD,
   ASYNC_COMPUTE,
   PRE_RENDER_P1,
   PRE_RENDER_P2,
   OPAQUE_RENDER,
   ALPHA_RENDER,
   POST_PROCESS,
#if CYD_DEBUG
   DEBUG_DRAW,
#endif
   POST_RENDER,
   UI,
   COUNT,
   LAST = COUNT - 1,
};

void Prepare();
CmdListHandle GetCommandList( Pass pass );
void Execute();
}
