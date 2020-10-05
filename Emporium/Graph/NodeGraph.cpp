#include <Graph/NodeGraph.h>

#include <cassert>

namespace EMP
{
void NodeGraph::reset() { m_nodes = {}; }

NodeGraph::NodeHandle NodeGraph::_addNode( std::unique_ptr<Node>&& newNode )
{
   int foundHandle = -1;

   for( uint32_t i = 0; i < MAX_AMOUNT_NODES; ++i )
   {
      auto& node = m_nodes[i];

      if( node == nullptr )
      {
         foundHandle = i;
         node        = std::move( newNode );
      }
   }

   assert( foundHandle >= 0 && "NodeGraph: Could not find free node slot" );

   return foundHandle;
}

void NodeGraph::_depthFirstSearch( NodeHandle root )
{
   const Node* rootNode = m_nodes[root].get();

   if( !rootNode ) return;

   _interpretNode( root, rootNode );

   for( uint32_t i = 0; i < rootNode->childrenCount; ++i )
   {
      _depthFirstSearch( rootNode->children[i] );
   }
}

void NodeGraph::_breathFirstSearch( NodeHandle /*root*/ ) {}

void NodeGraph::_interpretNode( NodeHandle handle, const Node* /*node*/ )
{
   // Read data
   printf( "NodeGraph: Reading node%d\n", handle );
}
}
