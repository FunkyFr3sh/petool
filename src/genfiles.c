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

#ifdef __linux__
#include <linux/limits.h>
#elif defined(__FreeBSD__ )
#include <limits.h>
#elif !defined(_WIN32)
#include <sys/syslimits.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MAX_PATH PATH_MAX
#define _mkdir(a) mkdir(a, 0777)
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

extern const char res_gitignore[];
extern const char res_build_cmd[];
extern const char res_readme_md[];
extern const char res_src_winmain_c[];
extern const char res_inc_app_h[];
extern const char res_inc_app_inc[];
extern const char res_inc_patch_h[];
extern const char res_inc_macros_datatypes_inc[];
extern const char res_inc_macros_extern_inc[];
extern const char res_inc_macros_extern_s[];
extern const char res_inc_macros_patch_h[];
extern const char res_inc_macros_patch_inc[];
extern const char res_inc_macros_setsym_h[];
extern const char res_inc_macros_watcall_inc[];
extern const char res_inc_macros_patch_s[];

void extract_resource(const char* src, char* file_path);

int genfiles(char *dir)
{
    int     ret   = EXIT_SUCCESS;

    static char buf[MAX_PATH];

    /* Optional files and examples - No error checking required here*/
    snprintf(buf, sizeof buf, "%s/.gitignore", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_gitignore, buf);

    snprintf(buf, sizeof buf, "%s/build.cmd", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_build_cmd, buf);

    snprintf(buf, sizeof buf, "%s/README.md", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_readme_md, buf);

    snprintf(buf, sizeof buf, "%s/src", dir);
    _mkdir(buf);

    snprintf(buf, sizeof buf, "%s/src/winmain.c", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_src_winmain_c, buf);

    snprintf(buf, sizeof buf, "%s/inc", dir);
    _mkdir(buf);

    snprintf(buf, sizeof buf, "%s/inc/app.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_app_h, buf);

    snprintf(buf, sizeof buf, "%s/inc/app.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_app_inc, buf);

    snprintf(buf, sizeof buf, "%s/inc/patch.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_patch_h, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros", dir);
    _mkdir(buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/datatypes.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_datatypes_inc, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/extern.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_extern_inc, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/extern.s", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_extern_s, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_patch_h, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_patch_inc, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/setsym.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_setsym_h, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/watcall.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_watcall_inc, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.s", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_patch_s, buf);

    return ret;
}

void extract_resource(const char* src, char* file_path)
{
    FILE* fp = fopen(file_path, "wb");
    if (fp)
    {
        fputs(src, fp);
        fclose(fp);
    }
}
