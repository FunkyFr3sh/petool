#include <windows.h>
#include "macros/patch.h"
#include "app.h"
#include "imports.h"


int WinMainCRTStartup(void);

// entry point
int start()
{
    if (!imports_init())
        return 0;

    return WinMainCRTStartup();
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    return app_start();
}
