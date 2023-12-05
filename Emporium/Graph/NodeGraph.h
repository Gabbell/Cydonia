#pragma once

#include <array>
#include <cstdint>
#include <memory>

namespace EMP
{
class NodeGraph
{
  public:
   using NodeHandle = uint32_t;

   NodeGraph()                                  = default;
   NodeGraph( const NodeGraph& )                = delete;
   NodeGraph( NodeGraph&& ) noexcept            = default;
   NodeGraph& operator=( const NodeGraph& )     = delete;
   NodeGraph& operator=( NodeGraph&& ) noexcept = default;
   virtual ~NodeGraph()                         = default;

   // Resets the whole graph
   void reset();

  protected:
   struct Node
   {
      static constexpr uint8_t MAX_BRANCHING         = 32;
      std::array<NodeHandle, MAX_BRANCHING> children = {};

      uint8_t childrenCount = 0;

      bool active : 1 = false;
   };

   NodeHandle _addNode( Node&& newNode );

   // Visit/Search functions
   void _depthFirstSearch( NodeHandle root );
   void _breathFirstSearch( NodeHandle root );

   // This function is called for each node visited during the different types of graph searches
   virtual void _interpretNode( NodeHandle handle, const Node& node );

  private:
   static constexpr uint32_t MAX_AMOUNT_NODES = 512;  // Increase this if needed
   static constexpr NodeHandle ROOT_INDEX     = 0;
   std::array<Node, MAX_AMOUNT_NODES> m_nodes;  // Graph nodes in contiguous memory
};
}
