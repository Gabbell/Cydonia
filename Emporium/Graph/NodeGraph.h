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

   NodeGraph()                       = default;
   NodeGraph( const NodeGraph& )     = delete;
   NodeGraph( NodeGraph&& ) noexcept = default;
   NodeGraph& operator=( const NodeGraph& ) = delete;
   NodeGraph& operator=( NodeGraph&& ) noexcept = default;
   virtual ~NodeGraph()                         = default;

   // Resets the whole graph
   virtual void reset();

  protected:
   struct Node
   {
      static constexpr uint8_t MAX_BRANCHING         = 32;
      std::array<NodeHandle, MAX_BRANCHING> children = {};

      uint8_t childrenCount = 0;
   };

   NodeHandle _addNode( std::unique_ptr<Node>&& newNode );

   // Visit/Search functions
   void _depthFirstSearch( NodeHandle root );
   void _breathFirstSearch( NodeHandle root );

  private:
   // This function is called for each node visited during the different types of graph searches
   virtual void _interpretNode( NodeHandle handle, const Node* node );

   static constexpr uint32_t MAX_AMOUNT_NODES = 512;  // Increase this if needed
   static constexpr NodeHandle ROOT_INDEX     = 0;
   std::array<std::unique_ptr<Node>, MAX_AMOUNT_NODES> m_nodes;  // Graph nodes in contiguous memory
};
}
