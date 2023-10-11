#include <windows.h>
#include "macros/patch.h"
#include "app.h"
#include "imports.h"

// entry point
void start()
{
    imports_init();
    app_start();
}
