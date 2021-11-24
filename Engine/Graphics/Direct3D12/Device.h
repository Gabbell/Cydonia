#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <string_view>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
struct ID3D12Device;
struct IDXGIAdapter1;
struct ID3D12CommandQueue;

#if defined( _DEBUG )
struct ID3D12DebugDevice;
#endif

namespace d3d12
{
class Factory;
class CommandList;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace d3d12
{
class Device final
{
  public:
   Device( IDXGIAdapter1* adapter );
   MOVABLE( Device );
   ~Device();

   CommandList* createCommandList( CYD::QueueUsageFlag usage, const std::string_view name );

  private:
   // =============================================================================================
   // Private Functions
   // =============================================================================================
   void _createDevice();
   void _createCommandQueues();
   void _createCommandAllocator();

   // =============================================================================================
   // Private Members
   // =============================================================================================
   ID3D12Device* m_d3d12Device;
   IDXGIAdapter1* m_adapter;

   std::vector<ID3D12CommandQueue*> m_commandQueues;

#if defined( _DEBUG )
   ID3D12DebugDevice* m_debugDevice;
#endif
};
}