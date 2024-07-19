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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#ifdef __linux__
#include <linux/limits.h>
#elif defined(__FreeBSD__ )
#include <limits.h>
#elif !defined(_WIN32)
#include <sys/syslimits.h>
#endif

#ifdef _WIN32
#include <direct.h>
#define MAX_PATH 260 /* including windows.h would conflict with pe.h */
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MAX_PATH PATH_MAX
#define _mkdir(a) mkdir(a, 0777)
#endif

#include "pe.h"
#include "cleanup.h"
#include "common.h"

extern bool g_sym_imports_enabled;

int genlds(int argc, char **argv);
int genmak(int argc, char **argv);
int gensym(int argc, char** argv, bool print_all);
int genfiles(char* dir);

int genprj(int argc, char **argv)
{
    int ret = EXIT_SUCCESS;
    static char base[MAX_PATH];
    static char buf[MAX_PATH];
    static char dir[MAX_PATH];
    char *cmd_argv[3] = { argv[0], argv[1], buf };

    FAIL_IF(argc < 2, "usage: petool genprj <image> [directory]\n");
    FAIL_IF(!file_exists(argv[1]), "input file missing\n");

    memset(base, 0, sizeof base);
    strncpy(base, file_basename(argv[1]), sizeof(base) - 1);
    char *p = strrchr(base, '.');
    if (p)
    {
        *p = '\0';
    }

    if (argc > 2)
    {
        memset(dir, 0, sizeof dir);
        strncpy(dir, argv[2], sizeof dir - 1);
    }
    else
    {
        snprintf(dir, sizeof dir, "%s", base);
    }

    printf("Input file      : %s\n", argv[1]);
    printf("Output directory: %s\n", dir);

    FAIL_IF_PERROR(_mkdir(dir) == -1, "Failed to create output directory");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/%s.dat", dir, base) < 0, "Failed to copy game executable - Path truncated\n");
    printf("Copying %s -> %s...\n", argv[1], buf);
    FAIL_IF(file_copy(argv[1], buf) != EXIT_SUCCESS, "Failed to copy file\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/%s.lds", dir, base) < 0, "Failed to create linker script - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(genlds(3, cmd_argv) != EXIT_SUCCESS, "Failed to create linker script\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/sym.cpp", dir) < 0, "Failed to create sym.cpp - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(gensym(3, cmd_argv, false) != EXIT_SUCCESS, "Failed to create sym.cpp\n");

    if (!g_sym_imports_enabled)
    {
        printf("WARNING: No LoadLibraryX / GetModuleHandleX found in executable, creating project WITHOUT working imports (No C++ support)\n");

        snprintf(buf, sizeof buf, "%s/sym.cpp", dir);
        printf("Generating %s with full import list...\n", buf);
        FAIL_IF(remove(buf) != 0, "Failed to delete old sym.cpp\n");
        FAIL_IF(gensym(3, cmd_argv, true) != EXIT_SUCCESS, "Failed to create sym.cpp\n");
    }

    FAIL_IF(snprintf(buf, sizeof buf, "%s/Makefile", dir) < 0, "Failed to create makefile - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(genmak(3, cmd_argv) != EXIT_SUCCESS, "Failed to create Makefile\n");

    printf("Extracting optional files and examples...\n");
    FAIL_IF(genfiles(dir) != EXIT_SUCCESS, "Failed to extract optional files and examples\n");

cleanup:
    return ret;
}
