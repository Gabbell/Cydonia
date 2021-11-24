#include <VKSandbox.h>
#include <D3D12Sandbox.h>

int main()
{
    {
        CYD::VKSandbox app(1920, 1080, "Cydonia Sandbox");
        app.startLoop();
    }

    // To see validation layer errors after destruction
    system("pause");
}
