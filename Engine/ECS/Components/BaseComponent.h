#pragma once

#include <Common/Include.h>
#include <Common/Assert.h>

#include <cstdint>

namespace CYD
{
class BaseComponent
{
  public:
   COPIABLE( BaseComponent );
   virtual ~BaseComponent() = default;

   // Every component has to have a set of init functions and implement this uninit virtual
   // function. The reason why the init functions are not pure virtual is because they can have any
   // number of arguments. We also need this kind of deferred initialization/uninitalization because
   // of the nature of the component pool. We cannot trust RAII when creating a component in the
   // component pool because if it becomes out of scope and the destructor is called (even inside an
   // assignment operator) the component could be invalidated. The compiler will tell you anyway if
   // you don't have the proper init function declared in your component.

   // virtual bool init( ... ) = 0; <-- you get the idea
   virtual void uninit() = 0;

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
