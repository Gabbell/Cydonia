#pragma once

#include <Common/Include.h>

#include <Graph/NodeGraph.h>

#include <Graphics/Handles/ResourceHandle.h>

namespace CYD::RenderGraph
{
enum class Pass
{
   LOAD,
   PRE_RENDER,
   OPAQUE_RENDER,
   ALPHA_RENDER,
#if CYD_DEBUG
   DEBUG_DRAW,
#endif
   POST_PROCESS,
   UI,
   COUNT
};

void Prepare();
CmdListHandle GetCommandList( Pass pass );
void Execute();
}
