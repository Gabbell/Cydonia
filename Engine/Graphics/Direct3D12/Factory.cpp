#include <Graphics/Direct3D12/Factory.h>

#include <Graphics/DirectX12.h>

#include <windows.h>

namespace d3d12
{
Factory::Factory()
{
   UINT factoryFlags = 0;

#if defined( _DEBUG )
   _createDebugController( factoryFlags );
#endif

   _createFactory( factoryFlags );
}

#if defined( _DEBUG )
void Factory::_createDebugController( uint32_t& factoryFlags )
{
   ID3D12Debug* debugInterface = nullptr;

   D3D12CALL( D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) ) );
   D3D12CALL( debugInterface->QueryInterface( IID_PPV_ARGS( &m_debugController ) ) );

   m_debugController->EnableDebugLayer();
   m_debugController->SetEnableGPUBasedValidation( true );

   factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

   debugInterface->Release();
}
#endif

void Factory::_createFactory( const uint32_t factoryFlags )
{
   D3D12CALL( CreateDXGIFactory2( factoryFlags, IID_PPV_ARGS( &m_factory ) ) );
}

Factory::~Factory()
{
   m_factory->Release();
   m_factory = nullptr;

#if defined( _DEBUG )
   m_debugController->Release();
   m_debugController = nullptr;
#endif
}
}