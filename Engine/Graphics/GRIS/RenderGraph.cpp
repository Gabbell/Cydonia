#include <Graphics/GRIS/RenderGraph.h>

#include <Common/Assert.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <Profiling.h>

namespace CYD::RenderGraph
{
static std::array<CmdListHandle, Pass::COUNT> s_cmdLists;

static const char* GetCommandListName( Pass pass )
{
   static constexpr char CMDLIST_NAMES[][32] = {
      "LOAD",
      "ASYNC_COMPUTE",
      "PRE_RENDER_P1",
      "PRE_RENDER_P2",
      "OPAQUE_RENDER",
      "ALPHA_RENDER",
      "POST_PROCESS",
#if CYD_DEBUG
      "DEBUG_DRAW",
#endif
      "POST_RENDER",
      "UI",
   };

   static_assert( ARRSIZE( CMDLIST_NAMES ) == Pass::COUNT );

   return CMDLIST_NAMES[pass];
}

static QueueUsageFlag GetCommandListType( Pass pass )
{
   static constexpr QueueUsageFlag CMDLIST_USAGE[] = {
      QueueUsage::TRANSFER,                        // LOAD
      QueueUsage::COMPUTE,                         // ASYNC_COMPUTE
      QueueUsage::GRAPHICS | QueueUsage::COMPUTE,  // PRE_RENDER_P1
      QueueUsage::GRAPHICS | QueueUsage::COMPUTE,  // PRE_RENDER_P2
      QueueUsage::GRAPHICS,                        // OPAQUE_RENDER
      QueueUsage::GRAPHICS,                        // ALPHA_RENDER
      QueueUsage::GRAPHICS | QueueUsage::COMPUTE,  // POST_PROCESS
#if CYD_DEBUG
      QueueUsage::GRAPHICS,  // DEBUG_DRAW
#endif
      QueueUsage::GRAPHICS | QueueUsage::COMPUTE,  // POST_RENDER
      QueueUsage::GRAPHICS,                        // UI
   };

   static_assert( ARRSIZE( CMDLIST_USAGE ) == Pass::COUNT );

   return CMDLIST_USAGE[pass];
}

void Prepare() { CYD_TRACE(); }

CmdListHandle GetCommandList( Pass pass )
{
   // TODO Specialized command lists
   if( !s_cmdLists[pass] )
   {
      if( pass == Pass::ASYNC_COMPUTE )
      {
         s_cmdLists[pass] = GRIS::CreateCommandList(
             GetCommandListType( pass ), GetCommandListName( pass ), true, false );
      }
      else
      {
         s_cmdLists[pass] = GRIS::CreateCommandList(
             GetCommandListType( pass ), GetCommandListName( pass ), false, true );
      }
   }

   return s_cmdLists[pass];
}

void Execute()
{
   CYD_TRACE();

   // Rest of command list synchronization
   bool seekFirst            = true;
   uint32_t prevCmdListIndex = 0;
   uint32_t cmdListIndex     = 0;
   for( ; cmdListIndex < s_cmdLists.size(); ++cmdListIndex )
   {
      const CmdListHandle prevCmdList = s_cmdLists[prevCmdListIndex];
      const CmdListHandle curCmdList  = s_cmdLists[cmdListIndex];

      if( cmdListIndex == Pass::ASYNC_COMPUTE || prevCmdListIndex == Pass::ASYNC_COMPUTE )
      {
         // Async compute is a free spirit and is synced manually
         continue;
      }

      if( seekFirst && curCmdList )
      {
         GRIS::SyncOnSwapchain( curCmdList );
         prevCmdListIndex = cmdListIndex;
         seekFirst        = false;
      }
      else if( s_cmdLists[cmdListIndex] )
      {
         GRIS::SyncOnCommandList( prevCmdList, curCmdList );
         prevCmdListIndex = cmdListIndex;
      }
   }

   const CmdListHandle lastCmdList = s_cmdLists[prevCmdListIndex];
   if( lastCmdList )
   {
      GRIS::SyncToSwapchain( lastCmdList );
   }

   // Submit all command lists
   for( uint32_t i = 0; i < Pass::COUNT; ++i )
   {
      const CmdListHandle cmdList = s_cmdLists[i];
      if( cmdList )
      {
         CYD_TRACE_S( GetCommandListName( static_cast<Pass>( i ) ) );
         GRIS::SubmitCommandList( cmdList );
         GRIS::DestroyCommandList( cmdList );
         s_cmdLists[i] = {};
      }
   }
}
}
