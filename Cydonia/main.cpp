#include "Core/App.h"

#include <cstdint>

static constexpr uint32_t width  = 1280;
static constexpr uint32_t height = 720;

int main()
{
   {
      cyd::App app( width, height, "It's not delivery, it's garbagio!" );
      app.startLoop();
   }

   // Dirty stuff to make sure we destroy properly and look at potential validation layer errors
   system( "pause" );

   return 0;
}
