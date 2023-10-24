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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>

#include "pe.h"
#include "cleanup.h"
#include "common.h"

uint32_t rva_to_offset(uint32_t address, PIMAGE_NT_HEADERS nt_hdr);

int gensym(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    FILE   *fh    = NULL;
    int8_t *image = NULL;
    FILE   *ofh   = stdout;

    FAIL_IF(argc < 2, "usage: petool gensym <image> [ofile]\n");

    uint32_t length;
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "rb"));

    if (argc > 2)
    {
        FAIL_IF(file_exists(argv[2]), "%s: output file already exists.\n", argv[2]);
        ofh = fopen(argv[2], "w");
        FAIL_IF_PERROR(ofh == NULL, "%s");
    }

    PIMAGE_DOS_HEADER dos_hdr = (void*)image;
    PIMAGE_NT_HEADERS nt_hdr = (void*)(image + dos_hdr->e_lfanew);

    fprintf(ofh, "%%include \"macros/setsym.inc\"\n\n\n");
    fprintf(ofh, "; vars\n\n\n");
    fprintf(ofh, "; functions\n\n");
    fprintf(ofh, "setcglob 0x%p, app_start\n\n", (void*)(nt_hdr->OptionalHeader.ImageBase + nt_hdr->OptionalHeader.AddressOfEntryPoint));
    fprintf(ofh, "; imports\n\n");

    FAIL_IF (nt_hdr->OptionalHeader.NumberOfRvaAndSizes < 2, "Not enough DataDirectories.\n");

    uint32_t offset = rva_to_offset(nt_hdr->OptionalHeader.ImageBase + nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress, nt_hdr);
    IMAGE_IMPORT_DESCRIPTOR *i = (void *)(image + offset);

    bool found_LoadLibraryA = false;
    bool found_GetModuleHandleA = false;
    bool found_GetProcAddress = false;
    bool found_MessageBoxA = false;

    while (i->OriginalFirstThunk) {
        char name[260] = { 0 };

        if (i->Name != 0) {
            strncpy(
                name,
                (char*)(image + rva_to_offset(nt_hdr->OptionalHeader.ImageBase + i->Name, nt_hdr)),
                sizeof name - 1
            );

            fprintf(ofh, "\n; %s\n", name);
        }

        PIMAGE_THUNK_DATA32 oft =
            (PIMAGE_THUNK_DATA32)(image + rva_to_offset(nt_hdr->OptionalHeader.ImageBase + i->OriginalFirstThunk, nt_hdr));

        PIMAGE_THUNK_DATA32 ft_rva =
            (PIMAGE_THUNK_DATA32)(nt_hdr->OptionalHeader.ImageBase + i->FirstThunk);

        while (oft->u1.AddressOfData)
        {
            PIMAGE_IMPORT_BY_NAME import =
                (PIMAGE_IMPORT_BY_NAME)(image + rva_to_offset(nt_hdr->OptionalHeader.ImageBase + oft->u1.AddressOfData, nt_hdr));

            if ((oft->u1.Ordinal & IMAGE_ORDINAL_FLAG32) == 0)
            {
                if (_strcmpi(name, "MSVCRT.dll") != 0) {
                    fprintf(ofh, "setcglob 0x%p, _imp__%s\n", (void*)&ft_rva->u1.Function, (const char*)import->Name);
                }

                if (strcmp((const char*)import->Name, "LoadLibraryA") == 0)
                    found_LoadLibraryA = true;

                if (strcmp((const char*)import->Name, "GetModuleHandleA") == 0)
                    found_GetModuleHandleA = true;

                if (strcmp((const char*)import->Name, "GetProcAddress") == 0)
                    found_GetProcAddress = true;

                if (strcmp((const char*)import->Name, "MessageBoxA") == 0)
                    found_MessageBoxA = true;
            }
            else
            {
                char* p = strrchr(name, '.');
                if (p)
                {
                    *p = '\0';
                }

                int ordinal = (oft->u1.Ordinal & ~IMAGE_ORDINAL_FLAG32) & 0xffff;

                fprintf(ofh, "setcglob 0x%p, _imp__%s_Ordinal_%d\n", (void*)&ft_rva->u1.Function, name, ordinal);
            }

            oft++;
            ft_rva++;
        }

        i++;
    }
    
    /* make sure the project builds without any manual changes */
    if (!found_LoadLibraryA)
        fprintf(ofh, "\n\nsetcglob 0x00000000, _imp__LoadLibraryA\n");

    if (!found_GetModuleHandleA)
        fprintf(ofh, "\n\nsetcglob 0x00000000, _imp__GetModuleHandleA\n");

    if (!found_GetProcAddress)
        fprintf(ofh, "\n\nsetcglob 0x00000000, _imp__GetProcAddress\n");

    if (!found_MessageBoxA)
        fprintf(ofh, "\n\nsetcglob 0x00000000, _imp__MessageBoxA\n");

cleanup:
    if (image) free(image);
    if (argc > 3)
    {
        if (ofh)   fclose(ofh);
    }
    return ret;
}