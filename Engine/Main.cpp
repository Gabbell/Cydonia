#include <Core/Applications/VKSandbox.h>

int main()
{
   {
      cyd::VKSandbox app( 1280, 720 );
      app.startLoop();
   }

   // To see validation layer errors after destruction
   system( "pause" );
}
