#include <windows.h>
#include "macros/patch.h"
#include "app.h"


int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    return app_start();
}
