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
