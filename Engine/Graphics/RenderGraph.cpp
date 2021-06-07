#include <Graphics/RenderGraph.h>

#include <Common/Assert.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD::RenderGraph
{
static std::vector<CmdListHandle> s_cmdLists;

void AddPass( CmdListHandle cmdList ) { s_cmdLists.push_back( cmdList ); }

void Execute()
{
   if( !s_cmdLists.empty() )
   {
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

      for( auto cmdList : s_cmdLists )
      {
         GRIS::DestroyCommandList( cmdList );
      }

      s_cmdLists.clear();
   }
}
}
