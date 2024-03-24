#include <Graphics/Direct3D12/DeviceManager.h>

#include <Graphics/DirectX12.h>

#include <Graphics/Direct3D12/Factory.h>
#include <Graphics/Direct3D12/Device.h>

namespace d3d12
{
DeviceManager::DeviceManager( const Factory& factory ) : m_factory( factory )
{
   REF( m_factory );

   IDXGIAdapter1* adapter;

   for( UINT adapterIdx = 0;
        DXGI_ERROR_NOT_FOUND != factory.getD3D12Factory()->EnumAdapters1( adapterIdx, &adapter );
        ++adapterIdx )
   {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1( &desc );

      if( desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE )
      {
         // Don't select the Basic Render Driver adapter.
         continue;
      }

      m_devices.emplace_back( std::make_unique<Device>( adapter ) );
   }
}

Device& DeviceManager::getMainDevice() const
{
   CYD_ASSERT( !m_devices.empty() && "DeviceManager: There were no devices" );
   return *m_devices[0];
}

DeviceManager::~DeviceManager() = default;
}