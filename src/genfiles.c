/*
 * Copyright (c) 2015 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif
#include <stdio.h>

void extract_resource(int id, char* file_path);

int genfiles(char *dir)
{
    int     ret   = EXIT_SUCCESS;

#ifdef _WIN32
    static char buf[MAX_PATH];

    /* Optional files and examples - No error checking required here*/
    snprintf(buf, sizeof buf, "%s/.gitignore", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 0, buf);

    snprintf(buf, sizeof buf, "%s/build.cmd", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 1, buf);

    snprintf(buf, sizeof buf, "%s/imports.asm", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 2, buf);

    snprintf(buf, sizeof buf, "%s/README.md", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 3, buf);

    snprintf(buf, sizeof buf, "%s/sym.asm", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 4, buf);

    snprintf(buf, sizeof buf, "%s/src", dir);
    _mkdir(buf);

    snprintf(buf, sizeof buf, "%s/src/example-fix.asm", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 5, buf);

    snprintf(buf, sizeof buf, "%s/src/winmain.c", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 6, buf);

    snprintf(buf, sizeof buf, "%s/inc", dir);
    _mkdir(buf);

    snprintf(buf, sizeof buf, "%s/inc/imports.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 7, buf);

    snprintf(buf, sizeof buf, "%s/inc/org.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 8, buf);

    snprintf(buf, sizeof buf, "%s/inc/org.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 9, buf);

    snprintf(buf, sizeof buf, "%s/inc/patch.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 17, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros", dir);
    _mkdir(buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/datatypes.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 10, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/extern.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 11, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/imports.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 12, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 13, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 14, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/setsym.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 15, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/watcall.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(1000 + 16, buf);
#endif

    return ret;
}

#ifdef _WIN32
void extract_resource(int id, char* file_path)
{
    HRSRC loc = FindResourceA(GetModuleHandleA(NULL), MAKEINTRESOURCE(id), RT_RCDATA);
    if (loc)
    {
        HGLOBAL res = LoadResource(GetModuleHandleA(NULL), loc);
        DWORD size = SizeofResource(GetModuleHandleA(NULL), loc);
        if (res && size > 0)
        {
            void* data = LockResource(res);
            if (data)
            {
                FILE* fp = fopen(file_path, "wb");
                if (fp)
                {
                    fwrite(data, 1, size, fp);
                    fclose(fp);
                }
            }
        }
    }
}
#endif
