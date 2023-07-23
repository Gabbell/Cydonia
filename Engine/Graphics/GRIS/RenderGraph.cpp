#include <Graphics/GRIS/RenderGraph.h>

#include <Common/Assert.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <Profiling.h>

namespace CYD::RenderGraph
{
static std::array<CmdListHandle, static_cast<uint32_t>( Pass::COUNT )> s_cmdLists;

static const char* GetCommandListName( Pass pass )
{
   static constexpr char CMDLIST_NAMES[][32] =
   { "LOAD",
     "PRE_RENDER",
     "OPAQUE_RENDER",
     "ALPHA_RENDER",
#if CYD_DEBUG
     "DEBUG_DRAW",
#endif
     "POST_PROCESS",
     "UI" };

   static_assert( ARRSIZE( CMDLIST_NAMES ) == static_cast<uint32_t>( Pass::COUNT ) );

   return CMDLIST_NAMES[static_cast<uint32_t>( pass )];
}

static QueueUsageFlag GetCommandListType( Pass pass )
{
   static constexpr QueueUsageFlag CMDLIST_USAGE[] =
   { QueueUsage::TRANSFER,
     QueueUsage::GRAPHICS | QueueUsage::COMPUTE,
     QueueUsage::GRAPHICS,
     QueueUsage::GRAPHICS,
#if CYD_DEBUG
     QueueUsage::GRAPHICS,
#endif
     QueueUsage::GRAPHICS | QueueUsage::COMPUTE,
     QueueUsage::GRAPHICS };

   static_assert( ARRSIZE( CMDLIST_USAGE ) == static_cast<uint32_t>( Pass::COUNT ) );

   return CMDLIST_USAGE[static_cast<uint32_t>( pass )];
}

void Prepare()
{
   for( uint32_t i = 0; i < static_cast<uint32_t>( Pass::COUNT ); ++i )
   {
      // TODO Specialized command lists
      s_cmdLists[i] = GRIS::CreateCommandList(
          GetCommandListType( Pass( i ) ), GetCommandListName( Pass( i ) ) );
   }
}

CmdListHandle GetCommandList( Pass pass ) { return s_cmdLists[static_cast<uint32_t>( pass )]; }

void Execute()
{
   CYD_TRACE( "RenderGraph Execute" );

   if( s_cmdLists.empty() )
   {
      CYD_ASSERT( !"Nothing to render?" );
      return;
   }

   GRIS::SyncOnSwapchain( s_cmdLists.front() );

   for( uint32_t i = 1; i < s_cmdLists.size(); ++i )
   {
      // Chain command lists together
      GRIS::SubmitCommandList( s_cmdLists[i - 1] );
      GRIS::SyncOnCommandList( s_cmdLists[i - 1], s_cmdLists[i] );
   }

   // We need to sync the last command list to the swapchain so that the swapchain knows when
   // the rendering is done
   GRIS::SyncToSwapchain( s_cmdLists.back() );
   GRIS::SubmitCommandList( s_cmdLists.back() );

   for( uint32_t i = 0; i < static_cast<uint32_t>( Pass::COUNT ); ++i )
   {
      GRIS::DestroyCommandList( s_cmdLists[i] );
      s_cmdLists[i] = {};
   }
}
}
