#pragma once

#include <Common/Include.h>

// ================================================================================================
// Forwards
// ================================================================================================
struct IDXGIFactory7;

#if defined( _DEBUG )
struct ID3D12Debug1;
struct ID3D12DebugDevice;
#endif

// ================================================================================================
// Definition
// ================================================================================================
namespace d3d12
{
class Factory final
{
  public:
   Factory();
   NON_COPIABLE( Factory );
   ~Factory();

   IDXGIFactory7* getD3D12Factory() const { return m_factory; }

  private:
   // =============================================================================================
   // Private Functions
   // =============================================================================================
   void _createFactory( const uint32_t factoryFlags );

#if defined( _DEBUG )
   void _createDebugController( uint32_t& factoryFlags );
#endif

   // =============================================================================================
   // Private Variables
   // =============================================================================================
   IDXGIFactory7* m_factory;

#if defined( _DEBUG )
   ID3D12Debug1* m_debugController;
#endif
};
}