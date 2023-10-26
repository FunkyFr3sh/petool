#include <windows.h>
#include "macros/patch.h"
#include "app.h"
#include "imports.h"

// entry point
void start()
{
    if (!imports_init())
        return;

    app_start();
}

7/* Required for c++ - Hook WinMain here and make sure to add the symbol for the real WinMain to sym.asm
CALL(0x00000000, _fake_WinMain);

int WinMainCRTStartup (void);
int APIENTRY fake_WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    return WinMainCRTStartup();
}
*/
