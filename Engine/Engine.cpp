#include <Applications/VKOceanDemo.h>

int main()
{
   {
      CYD::VKOceanDemo app( 1920, 1080, "GARBAGIO" );
      app.startLoop();
   }

   // To see validation layer errors after destruction
   system( "pause" );
}
