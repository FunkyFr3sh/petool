#include <windows.h>
#include <_mingw.h>
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
    HINSTANCE __UNUSED_PARAM(hInst), 
    HINSTANCE __UNUSED_PARAM(hInstPrev), 
    PSTR __UNUSED_PARAM(cmdline), 
    int __UNUSED_PARAM(cmdshow))
{
    return WinMainCRTStartup();
}
