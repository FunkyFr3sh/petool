#include <windows.h>
#include "macros/patch.h"
#include "app.h"
#include "imports.h"

/* Uncomment and replace 0x00000000 with the address where the original winmain is being called from and define app_WinMain in sym.asm */
//CALL(0x00000000, _fake_WinMain);

int APIENTRY fake_WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    imports_init();
    
    return app_WinMain(hInst, hInstPrev, cmdline, cmdshow);
}
