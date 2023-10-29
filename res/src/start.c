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

// Required for c++ - You must hook WinMain here and make sure you also update the address for the real WinMain in sym.asm
//CALL(0x00000000 <- <FIX_ME>, _fake_WinMain);

int WinMainCRTStartup(void);
int APIENTRY fake_WinMain(
    __attribute__((unused)) HINSTANCE hInst, 
    __attribute__((unused)) HINSTANCE hInstPrev, 
    __attribute__((unused)) PSTR cmdline, 
    __attribute__((unused)) int cmdshow)
{
    return WinMainCRTStartup();
}
