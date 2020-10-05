#pragma once

#include <Common/Include.h>
#include <Common/Assert.h>

#include <ECS/Components/ComponentTypes.h>

#include <cstdint>

namespace CYD
{
class BaseComponent
{
  public:
   COPIABLE( BaseComponent );
   virtual ~BaseComponent() = default;

   void setPoolIndex( int32_t poolIdx )
   {
      CYDASSERT( m_poolIdx == -1 && "BaseComponent: Pool index was already assgined" );
      m_poolIdx = poolIdx;
   }
   int32_t getPoolIndex() const noexcept { return m_poolIdx; }

  protected:
   BaseComponent() = default;

  private:
   int32_t m_poolIdx = -1;  // Index of component inside the pool
};
}
