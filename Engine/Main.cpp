#include <Core/Application.h>

static constexpr uint32_t width  = 1280;
static constexpr uint32_t height = 720;
static constexpr char title[]    = "It's not delivery, it's garbagio!";

int main()
{
   {
      cyd::Application app( width, height, title );
      app.startLoop();
   }

   // To see validation layer errors after destruction
   system( "pause" );
}