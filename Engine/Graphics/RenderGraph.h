#pragma once

#include <Common/Include.h>

#include <Graph/NodeGraph.h>

#include <Graphics/Handles/ResourceHandle.h>

namespace CYD::RenderGraph
{
void AddPass( CmdListHandle cmdList );
void Execute();
}
