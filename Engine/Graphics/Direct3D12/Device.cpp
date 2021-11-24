#include <Graphics/Direct3D12/Device.h>

#include <Graphics/DirectX12.h>

namespace d3d12
{
Device::Device( IDXGIAdapter1* adapter ) : m_adapter( adapter )
{
   _createDevice();
   _createCommandQueues();
   _createCommandAllocator();
}

void Device::_createDevice()
{
   D3D12CALL(
       D3D12CreateDevice( m_adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS( &m_d3d12Device ) ) );

#if defined( _DEBUG )
   D3D12CALL( m_d3d12Device->QueryInterface( &m_debugDevice ) );
#endif
}

void Device::_createCommandQueues()
{
   // We are using one direct queue for now
   D3D12_COMMAND_QUEUE_DESC queueDesc = {};
   queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
   queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

   ID3D12CommandQueue* commandQueue;
   D3D12CALL( m_d3d12Device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &commandQueue ) ) );

   m_commandQueues.push_back( commandQueue );
}

void Device::_createCommandAllocator() {}

CommandList* Device::createCommandList( CYD::QueueUsageFlag usage, const std::string_view name )
{
   return nullptr;
}

Device::~Device()
{
   m_d3d12Device->Release();
   m_d3d12Device = nullptr;

#if defined( _DEBUG )
   m_debugDevice->Release();
   m_debugDevice = nullptr;
#endif
}
}