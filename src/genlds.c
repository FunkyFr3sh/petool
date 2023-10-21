/*
 * Copyright (c) 2013 Toni Spets <toni.spets@iki.fi>
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
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "pe.h"
#include "cleanup.h"
#include "common.h"

int genlds(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret           = EXIT_SUCCESS;
    FILE   *fh            = NULL;
    FILE   *ofh           = stdout;
    int8_t *image         = NULL;
    char   inputname[256] = { '\0' };

    FAIL_IF(argc < 2, "usage: petool genlds <image> [ofile]\n");

    uint32_t length;
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "rb"));

    if (argc > 2)
    {
        FAIL_IF(file_exists(argv[2]), "%s: output file already exists.\n", argv[2]);
        ofh = fopen(argv[2], "w");
        FAIL_IF_PERROR(ofh == NULL, "%s");
    }

    fclose(fh);
    fh = NULL; // for cleanup

    PIMAGE_DOS_HEADER dos_hdr = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr = (void *)(image + dos_hdr->e_lfanew);

    FAIL_IF(length < 512,                            "File too small.\n");

    if (nt_hdr->Signature != IMAGE_NT_SIGNATURE)
    {
        nt_hdr = (void *)(image - 4);
        FAIL_IF(nt_hdr->FileHeader.Machine != 0x014C, "No valid signatures found.\n");
    }

    strncpy(inputname, file_basename(argv[1]), sizeof(inputname) - 1);
    char* p = strrchr(inputname, '.');
    if (p)
    {
        strncpy(p, ".dat", sizeof(inputname) - strlen(inputname) - 1);
    }

    fprintf(ofh, "/* GNU ld linker script for %s */\n", inputname);

    if (strcmp(argv[0], "genlds") == 0)
    {
        fprintf(ofh, "_start = 0x%"PRIX32";\n", nt_hdr->OptionalHeader.ImageBase + nt_hdr->OptionalHeader.AddressOfEntryPoint);
    }

    fprintf(ofh, "ENTRY(_start);\n");
    fprintf(ofh, "SECTIONS\n");
    fprintf(ofh, "{\n");

    bool idata_exists = false;
    uint16_t filln = 0;

    char align[64];
    sprintf(align, "ALIGN(0x%-4"PRIX32")", nt_hdr->OptionalHeader.SectionAlignment);

    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        const PIMAGE_SECTION_HEADER cur_sct = IMAGE_FIRST_SECTION(nt_hdr) + i;
        char buf[9];
        memset(buf, 0, sizeof buf);
        memcpy(buf, cur_sct->Name, 8);

        if (cur_sct->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA && !(cur_sct->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)) {
            fprintf(ofh, "    /DISCARD/                  : { %s(%s) }\n", inputname, buf);
            fprintf(ofh, "    %-15s   0x%-6"PRIX32" : { . = . + 0x%"PRIX32"; }\n", buf, cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase, cur_sct->Misc.VirtualSize ? cur_sct->Misc.VirtualSize : cur_sct->SizeOfRawData);
            continue;
        }

        /* resource section is not directly recompilable even if it doesn't move, use re2obj command instead */
        /* relocation section is not currently supported so we'll remove it */
        if (strcmp(buf, ".rsrc") == 0 || strcmp(buf, ".reloc") == 0) {
            fprintf(ofh, "    /DISCARD/                  : { %s(%s) }\n", inputname, buf);


            if (i < nt_hdr->FileHeader.NumberOfSections - 1) {
                sprintf(buf, "FILL%d", filln++);
                fprintf(ofh, "    %-15s   0x%-6"PRIX32" : { . = . + 0x%"PRIX32"; }\n", buf, cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase, cur_sct->Misc.VirtualSize ? cur_sct->Misc.VirtualSize : cur_sct->SizeOfRawData);
            }

            continue;
        }

        if (strcmp(buf, ".idata") == 0) {
            idata_exists = true;
        }

        if (cur_sct->Misc.VirtualSize > cur_sct->SizeOfRawData) {
            fprintf(ofh, "    %-15s   0x%-6"PRIX32" : { %s(%s) . = ALIGN(0x%"PRIX32"); }\n", buf, cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase, inputname, buf, nt_hdr->OptionalHeader.SectionAlignment);
            fprintf(ofh, "    .bss      %16s : { . = . + 0x%"PRIX32"; }\n", align, cur_sct->Misc.VirtualSize - cur_sct->SizeOfRawData);
            continue;
        }

        fprintf(ofh, "    %-15s   0x%-6"PRIX32" : { %s(%s) }\n", buf, cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase, inputname, buf);
    }

    fprintf(ofh, "\n");

    if (!idata_exists) {
        fprintf(ofh, "    .idata    %16s : { *(.idata) }\n\n", align);
    }

    fprintf(ofh, "    /DISCARD/                  : { *(.drectve) *(.rdata$zzz) *(.reloc) }\n");
    fprintf(ofh, "    .p_text   %16s : { *(.text) }\n", align);
    fprintf(ofh, "    .p_rdata  %16s : { *(.rdata) }\n", align);
    fprintf(ofh, "    .p_data   %16s : { *(.data) }\n", align);
    fprintf(ofh, "    .p_bss    %16s : { *(.bss) *(COMMON) }\n", align);
    fprintf(ofh, "    .rsrc     %16s : { *(.rsrc) }\n\n", align);
    fprintf(ofh, "    .patch    %16s : { *(.patch) }\n", align);

    fprintf(ofh, "}\n");

cleanup:
    if (image) free(image);
    if (fh)    fclose(fh);
    if (argc > 2)
    {
        if (ofh)   fclose(ofh);
    }
    return ret;
}
