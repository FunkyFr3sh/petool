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

#include "common.h"
#include "cleanup.h"

extern const char res_gitignore[];
extern const char res_build_cmd[];
extern const char res_readme_md[];
extern const char res_src_winmain_c[];
extern const char res_inc_app_h[];
extern const char res_inc_app_inc[];
extern const char res_inc_patch_h[];
extern const char res_inc_macros_datatypes_inc[];
extern const char res_inc_macros_datatypes_s[];
extern const char res_inc_macros_extern_inc[];
extern const char res_inc_macros_extern_s[];
extern const char res_inc_macros_patch_h[];
extern const char res_inc_macros_patch_inc[];
extern const char res_inc_macros_setsym_h[];
extern const char res_inc_macros_patch_s[];

int genfiles(char *dir)
{
    int     ret   = EXIT_SUCCESS;

    static char buf[MAX_PATH];

    snprintf(buf, sizeof buf, "%s/.gitignore", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_gitignore, buf) != EXIT_SUCCESS, "Failed to create .gitignore\n");

    snprintf(buf, sizeof buf, "%s/build.cmd", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_build_cmd, buf) != EXIT_SUCCESS, "Failed to create build.cmd\n");

    snprintf(buf, sizeof buf, "%s/README.md", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_readme_md, buf) != EXIT_SUCCESS, "Failed to create README.md\n");

    snprintf(buf, sizeof buf, "%s/src", dir);
    FAIL_IF_PERROR(_mkdir(buf) == -1, "Failed to create src subdirectory");

    snprintf(buf, sizeof buf, "%s/src/winmain.cpp", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_src_winmain_c, buf) != EXIT_SUCCESS, "Failed to create winmain.cpp\n");

    snprintf(buf, sizeof buf, "%s/inc", dir);
    FAIL_IF_PERROR(_mkdir(buf) == -1, "Failed to create inc subdirectory");

    snprintf(buf, sizeof buf, "%s/inc/app.h", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_app_h, buf) != EXIT_SUCCESS, "Failed to create app.h\n");

    snprintf(buf, sizeof buf, "%s/inc/app.inc", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_app_inc, buf) != EXIT_SUCCESS, "Failed to create app.inc\n");

    snprintf(buf, sizeof buf, "%s/inc/patch.h", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_patch_h, buf) != EXIT_SUCCESS, "Failed to create patch.h\n");

    snprintf(buf, sizeof buf, "%s/inc/macros", dir);
    FAIL_IF_PERROR(_mkdir(buf) == -1, "Failed to create macros subdirectory");

    snprintf(buf, sizeof buf, "%s/inc/macros/datatypes.inc", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_macros_datatypes_inc, buf) != EXIT_SUCCESS, "Failed to create datatypes.inc\n");

    snprintf(buf, sizeof buf, "%s/inc/macros/datatypes.s", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_macros_datatypes_s, buf) != EXIT_SUCCESS, "Failed to create datatypes.s\n");

    snprintf(buf, sizeof buf, "%s/inc/macros/extern.inc", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_macros_extern_inc, buf) != EXIT_SUCCESS, "Failed to create extern.inc\n");

    snprintf(buf, sizeof buf, "%s/inc/macros/extern.s", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_macros_extern_s, buf) != EXIT_SUCCESS, "Failed to create extern.s\n");

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.h", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_macros_patch_h, buf) != EXIT_SUCCESS, "Failed to create patch.h\n");

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.inc", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_macros_patch_inc, buf) != EXIT_SUCCESS, "Failed to create patch.inc\n");

    snprintf(buf, sizeof buf, "%s/inc/macros/setsym.h", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_macros_setsym_h, buf) != EXIT_SUCCESS, "Failed to create setsym.h\n");

    snprintf(buf, sizeof buf, "%s/inc/macros/patch.s", dir);
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_macros_patch_s, buf) != EXIT_SUCCESS, "Failed to create patch.s\n");

cleanup:
    return ret;
}
