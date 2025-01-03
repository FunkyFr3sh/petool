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

int genproxy_def(int argc, char** argv, bool forward);
int genproxy_exports(int argc, char** argv);
int genproxy_dllmain(int argc, char** argv, bool forward);
int genproxy_make(int argc, char** argv, bool forward);

extern const char res_proxy_readme_txt[];
extern const char res_proxy_dllmain_cpp[];
extern const char res_proxy_res_rc[];
extern const char res_proxy_vcxproj[];
extern const char res_build_cmd[];
extern const char res_inc_patch_h[];

int genproxy(int argc, char **argv)
{
    int ret = EXIT_SUCCESS;
    int8_t* image = NULL;
    uint32_t length;
    static char base[MAX_PATH];
    static char buf[MAX_PATH];
    static char dir[MAX_PATH];
    static char subdir[MAX_PATH];
    char *cmd_argv[3] = { argv[0], argv[1], buf };

    FAIL_IF(argc < 2, "usage: petool genproxy <image> [directory]\n");
    FAIL_IF_SILENT(open_and_read(NULL, &image, &length, argv[1], NULL));
    FAIL_IF(!is_supported_pe_image(image, length), "File is not a valid i386 Portable Executable (PE) image.\n");

    PIMAGE_DOS_HEADER dos_hdr = (void*)image;
    PIMAGE_NT_HEADERS nt_hdr = (void*)(image + dos_hdr->e_lfanew);

    FAIL_IF(nt_hdr->OptionalHeader.NumberOfRvaAndSizes < 1, "Not enough DataDirectories.\n");
    FAIL_IF(!nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress, "No export directory in dll\n");

    strncpy(base, file_basename_no_ext(argv[1]), sizeof(base) - 1);

    if (argc > 2)
    {
        memset(dir, 0, sizeof dir);
        strncpy(dir, argv[2], sizeof dir - 1);
    }
    else
    {
        FAIL_IF(snprintf(dir, sizeof dir, "%s-proxy", base) < 0, "Failed to create output directory - Path truncated\n")
    }

    printf("Input file      : %s\n", argv[1]);
    printf("Output directory: %s\n", dir);

    FAIL_IF_PERROR(_mkdir(dir) == -1, "Failed to create output directory");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/readme.txt", dir) < 0, "Failed to create readme.txt - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_proxy_readme_txt, buf) != EXIT_SUCCESS, "Failed to create readme.txt\n");


    FAIL_IF(snprintf(subdir, sizeof subdir, "%s/system", dir) < 0, "Failed to create output subdirectory - Path truncated\n");
    FAIL_IF_PERROR(_mkdir(subdir) == -1, "Failed to create output subdirectory");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/exports.def", subdir) < 0, "Failed to create exports.def - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(genproxy_def(3, cmd_argv, false) != EXIT_SUCCESS, "Failed to create exports.def\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/dllmain.cpp", subdir) < 0, "Failed to create dllmain.cpp - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(genproxy_dllmain(3, cmd_argv, false) != EXIT_SUCCESS, "Failed to create dllmain.cpp\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/exports.cpp", subdir) < 0, "Failed to create exports.cpp - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(genproxy_exports(3, cmd_argv) != EXIT_SUCCESS, "Failed to create exports.cpp\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/Makefile", subdir) < 0, "Failed to create makefile - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(genproxy_make(3, cmd_argv, false) != EXIT_SUCCESS, "Failed to create Makefile\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/build.cmd", subdir) < 0, "Failed to create build.cmd - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_build_cmd, buf) != EXIT_SUCCESS, "Failed to create build.cmd\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/patch.h", subdir) < 0, "Failed to create patch.h - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_patch_h, buf) != EXIT_SUCCESS, "Failed to create patch.h\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/%s.vcxproj", subdir, base) < 0, "Failed to create .vcxproj - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_proxy_vcxproj, buf) != EXIT_SUCCESS, "Failed to create .vcxproj\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/res.rc", subdir) < 0, "Failed to create res.rc - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_proxy_res_rc, buf) != EXIT_SUCCESS, "Failed to create res.rc\n");


    FAIL_IF(snprintf(subdir, sizeof subdir, "%s/local", dir) < 0, "Failed to create output subdirectory - Path truncated\n");
    FAIL_IF_PERROR(_mkdir(subdir) == -1, "Failed to create output subdirectory");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/exports.def", subdir) < 0, "Failed to create exports.def - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(genproxy_def(3, cmd_argv, true) != EXIT_SUCCESS, "Failed to create exports.def\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/dllmain.cpp", subdir) < 0, "Failed to create dllmain.cpp - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(genproxy_dllmain(3, cmd_argv, true) != EXIT_SUCCESS, "Failed to create dllmain.cpp\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/Makefile", subdir) < 0, "Failed to create makefile - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(genproxy_make(3, cmd_argv, true) != EXIT_SUCCESS, "Failed to create Makefile\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/build.cmd", subdir) < 0, "Failed to create build.cmd - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_build_cmd, buf) != EXIT_SUCCESS, "Failed to create build.cmd\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/patch.h", subdir) < 0, "Failed to create patch.h - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_inc_patch_h, buf) != EXIT_SUCCESS, "Failed to create patch.h\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/%s.vcxproj", subdir, base) < 0, "Failed to create .vcxproj - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_proxy_vcxproj, buf) != EXIT_SUCCESS, "Failed to create .vcxproj\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/res.rc", subdir) < 0, "Failed to create res.rc - Path truncated\n");
    printf("Generating %s...\n", buf);
    FAIL_IF(extract_resource(res_proxy_res_rc, buf) != EXIT_SUCCESS, "Failed to create res.rc\n");

    FAIL_IF(snprintf(buf, sizeof buf, "%s/%sx.dll", subdir, base) < 0, "Failed to copy original dll - Path truncated\n");
    printf("Copying %s -> %s...\n", argv[1], buf);
    FAIL_IF(file_copy(argv[1], buf) != EXIT_SUCCESS, "Failed to copy original dll\n");

cleanup:
    if (image) free(image);
    return ret;
}

int genproxy_def(int argc, char** argv, bool forward)
{
    // decleration before more meaningful initialization for cleanup
    int     ret = EXIT_SUCCESS;
    int8_t* image = NULL;
    FILE* ofh = NULL;
    uint32_t length;
    static char base[MAX_PATH];

    FAIL_IF(argc < 3, "usage: genproxy_exports <image> [ofile]\n");
    FAIL_IF_SILENT(open_and_read(NULL, &image, &length, argv[1], NULL));
    FAIL_IF(!is_supported_pe_image(image, length), "File is not a valid i386 Portable Executable (PE) image.\n");

    FAIL_IF(file_exists(argv[2]), "%s: output file already exists.\n", argv[2]);
    ofh = fopen(argv[2], "w");
    FAIL_IF_PERROR(ofh == NULL, "%s");

    PIMAGE_DOS_HEADER dos_hdr = (void*)image;
    PIMAGE_NT_HEADERS nt_hdr = (void*)(image + dos_hdr->e_lfanew);

    FAIL_IF(nt_hdr->OptionalHeader.NumberOfRvaAndSizes < 1, "Not enough DataDirectories.\n");
    FAIL_IF(!nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress, "No export directory in dll\n");

    uint32_t offset = rva_to_offset(nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress, nt_hdr);
    IMAGE_EXPORT_DIRECTORY* export_dir = (void*)(image + offset);

    fprintf(ofh, "LIBRARY %s\n", file_basename(argv[1]));
    fprintf(ofh, "\n");
    fprintf(ofh, "EXPORTS\n");

    strncpy(base, file_basename_no_ext(argv[1]), sizeof(base) - 1);

    uint32_t* names = (uint32_t*)(image + rva_to_offset(export_dir->AddressOfNames, nt_hdr));
    uint16_t* ordinals = (uint16_t*)(image + rva_to_offset(export_dir->AddressOfNameOrdinals, nt_hdr));

    for (uint32_t i = 0; i < export_dir->NumberOfFunctions; i++)
    {
        char* name = NULL;

        for (uint32_t x = 0; x < export_dir->NumberOfNames; x++)
        {
            if (ordinals[x] == i)
            {
                name = (char*)(image + rva_to_offset(names[x], nt_hdr));
                break;
            }
        }

        if (name)
        {
            bool is_private =
                strcmp(name, "DllCanUnloadNow") == 0 ||
                strcmp(name, "DllGetClassObject") == 0 ||
                strcmp(name, "DllGetClassFactoryFromClassString") == 0 ||
                strcmp(name, "DllGetDocumentation") == 0 ||
                strcmp(name, "DllInitialize") == 0 ||
                strcmp(name, "DllInstall") == 0 ||
                strcmp(name, "DllRegisterServer") == 0 ||
                strcmp(name, "DllRegisterServerEx") == 0 ||
                strcmp(name, "DllRegisterServerExW") == 0 ||
                strcmp(name, "DllUnload") == 0 ||
                strcmp(name, "DllUnregisterServer") == 0 ||
                strcmp(name, "RasCustomDeleteEntryNotify") == 0 ||
                strcmp(name, "RasCustomDial") == 0 ||
                strcmp(name, "RasCustomDialDlg") == 0 ||
                strcmp(name, "RasCustomEntryDlg") == 0;

            if (is_private)
            {
                if (forward)
                {
                    fprintf(ofh, "    %-40s = %sx.%-40s PRIVATE\n", name, base, name);
                }
                else
                {
                    fprintf(ofh, "    %-40s = __export_%-3u PRIVATE\n", name, i);
                }
            }
            else
            {
                if (forward)
                {
                    fprintf(ofh, "    %-40s = %sx.%-40s @%u\n", name, base, name, export_dir->Base + i);
                }
                else
                {
                    fprintf(ofh, "    %-40s = __export_%-3u @%u\n", name, i, export_dir->Base + i);
                }
            }
        }
        else
        {
            if (forward)
            {
                // forwaring to ordinal is not supported by GNU ld right now
                //fprintf(ofh, "    %-40s = %sx.#%-40u @%u NONAME\n", name, base, export_dir->Base + i, export_dir->Base + i);
            }
            else
            {
                fprintf(ofh, "    __export_%-31u = __export_%-3u @%u NONAME\n", i, i, export_dir->Base + i);
            }
        }
    }

cleanup:
    if (image) free(image);
    if (ofh) fclose(ofh);
    return ret;
}

int genproxy_exports(int argc, char** argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret = EXIT_SUCCESS;
    int8_t* image = NULL;
    FILE* ofh = NULL;
    uint32_t length;

    FAIL_IF(argc < 3, "usage: genproxy_exports <image> [ofile]\n");
    FAIL_IF_SILENT(open_and_read(NULL, &image, &length, argv[1], NULL));
    FAIL_IF(!is_supported_pe_image(image, length), "File is not a valid i386 Portable Executable (PE) image.\n");

    FAIL_IF(file_exists(argv[2]), "%s: output file already exists.\n", argv[2]);
    ofh = fopen(argv[2], "w");
    FAIL_IF_PERROR(ofh == NULL, "%s");

    PIMAGE_DOS_HEADER dos_hdr = (void*)image;
    PIMAGE_NT_HEADERS nt_hdr = (void*)(image + dos_hdr->e_lfanew);

    FAIL_IF(nt_hdr->OptionalHeader.NumberOfRvaAndSizes < 1, "Not enough DataDirectories.\n");
    FAIL_IF(!nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress, "No export directory in dll\n");

    uint32_t offset = rva_to_offset(nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress, nt_hdr);
    IMAGE_EXPORT_DIRECTORY* export_dir = (void*)(image + offset);

    fprintf(ofh, "// petool genproxy - https://github.com/FunkyFr3sh/petool\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "#include <windows.h>\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "FARPROC g_exports[%u];\n", export_dir->NumberOfFunctions);
    fprintf(ofh, "static INIT_ONCE g_exports_init_once;\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "static void exports_init()\n");
    fprintf(ofh, "{\n");
    fprintf(ofh, "    WCHAR path[MAX_PATH];\n");
    fprintf(ofh, "    if (!GetSystemDirectoryW(path, _countof(path)))\n");
    fprintf(ofh, "        return;\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "    if (wcscat_s(path, L\"\\\\%s\") != 0)\n", file_basename(argv[1]));
    fprintf(ofh, "        return;\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "    HMODULE dll = LoadLibraryW(path);\n");
    fprintf(ofh, "    if (!dll)\n");
    fprintf(ofh, "        return;\n");
    fprintf(ofh, "\n");

    uint32_t* names = (uint32_t*)(image + rva_to_offset(export_dir->AddressOfNames, nt_hdr));
    uint16_t* ordinals = (uint16_t*)(image + rva_to_offset(export_dir->AddressOfNameOrdinals, nt_hdr));
    uint32_t base = export_dir->Base;

    for (uint32_t i = 0; i < export_dir->NumberOfFunctions; i++)
    {
        char* name = NULL;

        for (uint32_t x = 0; x < export_dir->NumberOfNames; x++)
        {
            if (ordinals[x] == i)
            {
                name = (char*)(image + rva_to_offset(names[x], nt_hdr));
                break;
            }
        }

        int align = snprintf(0, 0, "%u", i);
        align = 3 - (align > 3 ? 3 : align);

        if (name)
        {
            fprintf(ofh, "    g_exports[%u]%*s = GetProcAddress(dll, \"%s\");\n", i, align, "", name);
        }
        else
        {
            fprintf(ofh, "    g_exports[%u]%*s = GetProcAddress(dll, MAKEINTRESOURCEA(%u));\n", i, align, "", base + i);
        }
    }

    fprintf(ofh, "}\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "#if defined(_MSC_VER)\n");
    fprintf(ofh, "#define ASM_JMP(a) __asm { jmp g_exports[a*4] }\n");
    fprintf(ofh, "#define NAKED __declspec(naked)\n");
    fprintf(ofh, "#define NOINLINE __declspec(noinline)\n");
    fprintf(ofh, "#else\n");
    fprintf(ofh, "#define ASM_JMP(a) __asm(\"jmp _g_exports[\" #a \"*4]\")\n");
    fprintf(ofh, "#define NAKED __attribute__((naked))\n");
    fprintf(ofh, "#define NOINLINE __attribute__((noinline))\n");
    fprintf(ofh, "#endif\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "static NOINLINE void exports_init_once()\n");
    fprintf(ofh, "{\n");
    fprintf(ofh, "    BOOL pending;\n");
    fprintf(ofh, "    if (InitOnceBeginInitialize(&g_exports_init_once, 0, &pending, NULL) && pending)\n");
    fprintf(ofh, "    {\n");
    fprintf(ofh, "        exports_init();\n");
    fprintf(ofh, "        InitOnceComplete(&g_exports_init_once, 0, NULL);\n");
    fprintf(ofh, "    }\n");
    fprintf(ofh, "}\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "#define CREATE_EXPORT(a) \\\n");
    fprintf(ofh, "    EXTERN_C NAKED void __export_##a() { \\\n");
    fprintf(ofh, "        exports_init_once(); \\\n");
    fprintf(ofh, "        ASM_JMP(a); \\\n");
    fprintf(ofh, "    }\n");
    fprintf(ofh, "\n");

    for (uint32_t i = 0; i < export_dir->NumberOfFunctions; i++)
    {
        fprintf(ofh, "CREATE_EXPORT(%u)\n", i);
    }

cleanup:
    if (image) free(image);
    if (ofh) fclose(ofh);
    return ret;
}

int genproxy_dllmain(int argc, char** argv, bool forward)
{
    // decleration before more meaningful initialization for cleanup
    int     ret = EXIT_SUCCESS;
    int8_t* image = NULL;
    FILE* ofh = NULL;
    uint32_t length;
    static char base[MAX_PATH];

    FAIL_IF(argc < 3, "usage: genproxy_dllmain <image> [ofile]\n");
    FAIL_IF_SILENT(open_and_read(NULL, &image, &length, argv[1], NULL));
    FAIL_IF(!is_supported_pe_image(image, length), "File is not a valid i386 Portable Executable (PE) image.\n");

    FAIL_IF(file_exists(argv[2]), "%s: output file already exists.\n", argv[2]);
    ofh = fopen(argv[2], "w");
    FAIL_IF_PERROR(ofh == NULL, "%s");

    PIMAGE_DOS_HEADER dos_hdr = (void*)image;
    PIMAGE_NT_HEADERS nt_hdr = (void*)(image + dos_hdr->e_lfanew);

    FAIL_IF(nt_hdr->OptionalHeader.NumberOfRvaAndSizes < 1, "Not enough DataDirectories.\n");
    FAIL_IF(!nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress, "No export directory in dll\n");

    uint32_t offset = rva_to_offset(nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress, nt_hdr);
    IMAGE_EXPORT_DIRECTORY* export_dir = (void*)(image + offset);

    fprintf(ofh, "#include <windows.h>\n");
    fprintf(ofh, "#include \"patch.h\"\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)\n");
    fprintf(ofh, "{\n");
    fprintf(ofh, "    switch(fdwReason)\n");
    fprintf(ofh, "    {\n");
    fprintf(ofh, "    case DLL_PROCESS_ATTACH:\n");
    fprintf(ofh, "        break;\n");
    fprintf(ofh, "    case DLL_PROCESS_DETACH:\n");
    fprintf(ofh, "        break;\n");
    fprintf(ofh, "    }\n");
    fprintf(ofh, "    return TRUE;\n");
    fprintf(ofh, "}\n");

    if (!forward)
    {
        goto cleanup;
    }

    fprintf(ofh, "\n");
    fprintf(ofh, "// petool genproxy - https://github.com/FunkyFr3sh/petool\n");
    fprintf(ofh, "#if defined(_MSC_VER)\n");

    strncpy(base, file_basename_no_ext(argv[1]), sizeof(base) - 1);

    uint32_t* names = (uint32_t*)(image + rva_to_offset(export_dir->AddressOfNames, nt_hdr));
    uint16_t* ordinals = (uint16_t*)(image + rva_to_offset(export_dir->AddressOfNameOrdinals, nt_hdr));

    for (uint32_t i = 0; i < export_dir->NumberOfFunctions; i++)
    {
        char* name = NULL;

        for (uint32_t x = 0; x < export_dir->NumberOfNames; x++)
        {
            if (ordinals[x] == i)
            {
                name = (char*)(image + rva_to_offset(names[x], nt_hdr));
                break;
            }
        }

        if (name)
        {
            bool is_private =
                strcmp(name, "DllCanUnloadNow") == 0 ||
                strcmp(name, "DllGetClassObject") == 0 ||
                strcmp(name, "DllGetClassFactoryFromClassString") == 0 ||
                strcmp(name, "DllGetDocumentation") == 0 ||
                strcmp(name, "DllInitialize") == 0 ||
                strcmp(name, "DllInstall") == 0 ||
                strcmp(name, "DllRegisterServer") == 0 ||
                strcmp(name, "DllRegisterServerEx") == 0 ||
                strcmp(name, "DllRegisterServerExW") == 0 ||
                strcmp(name, "DllUnload") == 0 ||
                strcmp(name, "DllUnregisterServer") == 0 ||
                strcmp(name, "RasCustomDeleteEntryNotify") == 0 ||
                strcmp(name, "RasCustomDial") == 0 ||
                strcmp(name, "RasCustomDialDlg") == 0 ||
                strcmp(name, "RasCustomEntryDlg") == 0;

            if (is_private)
            {
                fprintf(ofh, "#pragma comment(linker, \"/export:%s=%sx.%s,PRIVATE\")\n", name, base, name);
            }
            else
            {
                fprintf(ofh, "#pragma comment(linker, \"/export:%s=%sx.%s,@%u\")\n", name, base, name, export_dir->Base + i);
            }
        }
        else
        {
            uint32_t ord = export_dir->Base + i;
            fprintf(ofh, "#pragma comment(linker, \"/export:__export_%u=%sx.#%u,@%u,NONAME\")\n", i, base, ord, ord);
        }
    }

    fprintf(ofh, "#endif\n");

cleanup:
    if (image) free(image);
    if (ofh) fclose(ofh);
    return ret;
}

int genproxy_make(int argc, char** argv, bool forward)
{
    // decleration before more meaningful initialization for cleanup
    int     ret = EXIT_SUCCESS;
    FILE* ofh = NULL;

    FAIL_IF(argc < 3, "usage: genproxy_make <image> [ofile]\n");

    FAIL_IF(file_exists(argv[2]), "%s: output file already exists.\n", argv[2]);
    ofh = fopen(argv[2], "w");
    FAIL_IF_PERROR(ofh == NULL, "%s");

    fprintf(ofh, "-include config.mk\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "TARGET      ?= %s\n", file_basename(argv[1]));
    fprintf(ofh, "\n");
    fprintf(ofh, "LDFLAGS     ?= -Wl,--enable-stdcall-fixup -s -shared -static\n");
    fprintf(ofh, "CFLAGS      ?= -masm=intel -O2 -Wall -march=pentium4 -D _WIN32_WINNT=0x0600\n");
    fprintf(ofh, "CXXFLAGS    ?= -masm=intel -O2 -Wall -march=pentium4 -D _WIN32_WINNT=0x0600\n");
    fprintf(ofh, "LIBS        ?= \n");
    fprintf(ofh, "\n");
    fprintf(ofh, "CC           = i686-w64-mingw32-gcc\n");
    fprintf(ofh, "CXX          = i686-w64-mingw32-g++\n");
    fprintf(ofh, "STRIP       ?= i686-w64-mingw32-strip\n");
    fprintf(ofh, "WINDRES     ?= i686-w64-mingw32-windres\n");
    fprintf(ofh, "\n");

    if (forward)
    {
        fprintf(ofh, "OBJS = dllmain.o res.o\n");
    }
    else
    {
        fprintf(ofh, "OBJS = dllmain.o exports.o res.o\n");
    }

    fprintf(ofh, "\n");
    fprintf(ofh, ".PHONY: all clean\n");
    fprintf(ofh, "all: $(TARGET)\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "%%.o: %%.rc\n");
    fprintf(ofh, "	$(WINDRES) -J rc $< $@ || windres -J rc $< $@\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "$(TARGET): $(OBJS)\n");
    fprintf(ofh, "	$(CXX) $(LDFLAGS) -o $@ $^ exports.def -L. $(LIBS)\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "clean:\n");
    fprintf(ofh, "	$(RM) $(OBJS) $(TARGET) || del $(TARGET) $(subst /,\\\\,$(OBJS))\n");

cleanup:
    if (ofh) fclose(ofh);
    return ret;
}
