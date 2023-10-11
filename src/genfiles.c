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

#ifdef __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#define _mkdir(a) mkdir(a, 0777)
#endif

#include <stdlib.h>
#include <stdio.h>

extern const char res_gitignore[];
extern const char res_build_cmd[];
extern const char res_imports_asm[];
extern const char res_readme_md[];
extern const char res_src_example_fix_asm[];
extern const char res_src_winmain_c[];
extern const char res_inc_imports_h[];
extern const char res_inc_app_h[];
extern const char res_inc_app_inc[];
extern const char res_inc_patch_h[];
extern const char res_inc_macros_datatypes_inc[];
extern const char res_inc_macros_extern_inc[];
extern const char res_inc_macros_imports_inc[];
extern const char res_inc_macros_patch_h[];
extern const char res_inc_macros_patch_inc[];
extern const char res_inc_macros_setsym_inc[];
extern const char res_inc_macros_watcall_inc[];

    __asm(".data;"
    "_res_gitignore:"
    "res_gitignore:"
    ".incbin \"res/.gitignore\";"
    ".byte 0;"
    "_res_build_cmd:"
    "res_build_cmd:"
    ".incbin \"res/build.cmd\";"
    ".byte 0;"
    "_res_imports_asm:"
    "res_imports_asm:"
    ".incbin \"res/imports.asm\";"
    ".byte 0;"
    "_res_readme_md:"
    "res_readme_md:"
    ".incbin \"res/readme.md\";"
    ".byte 0;"
    "_res_src_example_fix_asm:"
    "res_src_example_fix_asm:"
    ".incbin \"res/src/example-fix.asm\";"
    ".byte 0;"
    "_res_src_winmain_c:"
    "res_src_winmain_c:"
    ".incbin \"res/src/winmain.c\";"
    ".byte 0;"
    "_res_inc_imports_h:"
    "res_inc_imports_h:"
    ".incbin \"res/inc/imports.h\";"
    ".byte 0;"
    "_res_inc_app_h:"
    "res_inc_app_h:"
    ".incbin \"res/inc/app.h\";"
    ".byte 0;"
    "_res_inc_app_inc:"
    "res_inc_app_inc:"
    ".incbin \"res/inc/app.inc\";"
    ".byte 0;"
    "_res_inc_patch_h:"
    "res_inc_patch_h:"
    ".incbin \"res/inc/patch.h\";"
    ".byte 0;"
    "_res_inc_macros_datatypes_inc:"
    "res_inc_macros_datatypes_inc:"
    ".incbin \"res/inc/macros/datatypes.inc\";"
    ".byte 0;"
    "_res_inc_macros_extern_inc:"
    "res_inc_macros_extern_inc:"
    ".incbin \"res/inc/macros/extern.inc\";"
    ".byte 0;"
    "_res_inc_macros_imports_inc:"
    "res_inc_macros_imports_inc:"
    ".incbin \"res/inc/macros/imports.inc\";"
    ".byte 0;"
    "_res_inc_macros_patch_h:"
    "res_inc_macros_patch_h:"
    ".incbin \"res/inc/macros/patch.h\";"
    ".byte 0;"
    "_res_inc_macros_patch_inc:"
    "res_inc_macros_patch_inc:"
    ".incbin \"res/inc/macros/patch.inc\";"
    ".byte 0;"
    "_res_inc_macros_setsym_inc:"
    "res_inc_macros_setsym_inc:"
    ".incbin \"res/inc/macros/setsym.inc\";"
    ".byte 0;"
    "_res_inc_macros_watcall_inc:"
    "res_inc_macros_watcall_inc:"
    ".incbin \"res/inc/macros/watcall.inc\";"
    ".byte 0"
    );

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

    snprintf(buf, sizeof buf, "%s/imports.asm", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_imports_asm, buf);

    snprintf(buf, sizeof buf, "%s/README.md", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_readme_md, buf);

    snprintf(buf, sizeof buf, "%s/src", dir);
    _mkdir(buf);

    snprintf(buf, sizeof buf, "%s/src/example-fix.asm", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_src_example_fix_asm, buf);

    snprintf(buf, sizeof buf, "%s/src/winmain.c", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_src_winmain_c, buf);

    snprintf(buf, sizeof buf, "%s/inc", dir);
    _mkdir(buf);

    snprintf(buf, sizeof buf, "%s/inc/imports.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_imports_h, buf);

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

    snprintf(buf, sizeof buf, "%s/inc/macros/imports.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_imports_inc, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.h", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_patch_h, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_patch_inc, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/setsym.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_setsym_inc, buf);

    snprintf(buf, sizeof buf, "%s/inc/macros/watcall.inc", dir);
    printf("Generating %s...\n", buf);
    extract_resource(res_inc_macros_watcall_inc, buf);

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
