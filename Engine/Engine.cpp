#include <Applications/VKSandbox.h>

int main()
{
   {
      cyd::VKSandbox app;

      app.init( 1280, 720, "VKSandbox" );
      app.startLoop();
   }

   // To see validation layer errors after destruction
   system( "pause" );
}
