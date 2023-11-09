#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
A framebuffer is only a container, it does not control the lifetime of its internal textures
*/
namespace CYD
{
class Framebuffer
{
  public:
   Framebuffer() = default;
   Framebuffer( uint32_t width, uint32_t height ) : m_width( width ), m_height( height ) {}
   COPIABLE( Framebuffer );
   ~Framebuffer() = default;

   // ================================================================================================
   enum Index
   {
      COLOR,
      DEPTH
   };

   static constexpr uint32_t MAX_TARGETS = 8;

   struct RenderTarget
   {
      TextureHandle texture = {};
      Access nextAccess     = Access::UNDEFINED;
      ClearValue clearValue = {};
   };

   using RenderTargets = std::array<RenderTarget, MAX_TARGETS>;

   // ================================================================================================
   void resize( uint32_t width, uint32_t height );

   void attach(
       uint32_t idx,
       TextureHandle texture,
       Access nextAccess,
       const ClearValue& clearValue = {} );
   void replace(
       uint32_t idx,
       TextureHandle texture,
       Access nextAccess,
       const ClearValue& clearValue = {} );
   void detach( uint32_t idx );

   void bind( CmdListHandle cmdList, uint32_t idx, uint32_t binding, uint32_t set = 0 ) const;

   // Getters
   // ================================================================================================
   bool isValid() const;

   uint32_t getWidth() const { return m_width; }
   uint32_t getHeight() const { return m_height; }
   const RenderTargets& getRenderTargets() const { return m_targets; }
   bool shouldClearAll() const { return m_clearAll; }

   // Setters
   // ================================================================================================
   void setClearAll( bool shouldClear );

  private:
   bool m_clearAll = false;

   uint32_t m_width  = 0;
   uint32_t m_height = 0;

   RenderTargets m_targets;
};
}