#include <windows.h>
#include "macros/patch.h"
#include "app.h"
#include "imports.h"


int WinMainCRTStartup(void);

// Required for c++ - You must hook WinMain here and make sure you also update the address for the real WinMain in sym.c

//CALL(0x00000000 <- <FIX_ME>, _fake_WinMain);
int APIENTRY fake_WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    return WinMainCRTStartup();
}

// entry point
int start()
{
    if (!imports_init())
        return 0;

    return OriginalCRTStartup();
}
