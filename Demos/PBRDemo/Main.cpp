#include <PBRDemo.h>

int main()
{
   {
      CYD::PBRDemo app( 2560, 1440, "CYDONIA" );
      app.startLoop();
   }

#if CYD_DEBUG
   // To see validation layer errors after destruction
   system( "pause" );
#endif
}