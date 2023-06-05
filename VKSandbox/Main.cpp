#include <VKSandbox.h>

int main()
{
    {
        CYD::VKSandbox app(2560, 1440, "Cydonia Sandbox");
        app.startLoop();
    }

    // To see validation layer errors after destruction
    system("pause");
}