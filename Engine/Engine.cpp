#include <Applications/VKSandbox.h>

int main()
{
   {
      cyd::VKSandbox app;

      app.init( 1920, 1080, "VKSandbox" );
      app.startLoop();
   }

   // To see validation layer errors after destruction
   system( "pause" );
}
