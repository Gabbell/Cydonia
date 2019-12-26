#pragma once

#include <Common/Include.h>

#include <ECS/Entities/Entity.h>

#include <limits>

namespace cyd
{
class BaseComponent
{
  public:
   BaseComponent() = default;
   NON_COPIABLE( BaseComponent );
   virtual ~BaseComponent() = default;

   virtual bool init( const void* pDescription ) = 0;

   void setEntityHandle( EntityHandle handle ) { m_entityHandle = handle; }
   EntityHandle getEntityHandle() const { return m_entityHandle; }

   void setPoolIndex( size_t index ) { m_poolIdx = index; }
   size_t getPoolIndex() const { return m_poolIdx; }

  private:
   static constexpr size_t INVALID_POOL_IDX = std::numeric_limits<size_t>::max();

   // The entity's handle to which this component belongs to
   EntityHandle m_entityHandle = Entity::INVALID_HANDLE;

   // Index in the contiguous block of allocated memory for this component
   size_t m_poolIdx = INVALID_POOL_IDX;
};
}
