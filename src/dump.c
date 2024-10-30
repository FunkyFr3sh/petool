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

int dump(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    FILE   *fh    = NULL;
    int8_t *image = NULL;

    FAIL_IF(argc < 2, "usage: petool dump <image>\n");

    uint32_t length;
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "rb"));

    fclose(fh);
    fh = NULL; // for cleanup

    PIMAGE_DOS_HEADER dos_hdr = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr = (void *)(image + dos_hdr->e_lfanew);

    FAIL_IF(length < 512, "File too small.\n");
    FAIL_IF(dos_hdr->e_lfanew > length - 4, "Unknown file type.\n");

    if (dos_hdr->e_magic == IMAGE_DOS_SIGNATURE)
    {
        if (nt_hdr->Signature != IMAGE_NT_SIGNATURE)
        {
            uint32_t exe_start = dos_hdr->e_cparhdr * 16L;
            uint32_t exe_end = dos_hdr->e_cp * 512L - (dos_hdr->e_cblp ? 512L - dos_hdr->e_cblp : 0);

            printf("DOS Header:\n");
            printf(" e_magic:    %04X\n", dos_hdr->e_magic);
            printf(" e_cblp:     %04X\n", dos_hdr->e_cblp);
            printf(" e_cp:       %04X\n", dos_hdr->e_cp);
            printf(" e_crlc:     %04X\n", dos_hdr->e_crlc);
            printf(" e_cparhdr:  %04X\n", dos_hdr->e_cparhdr);
            printf(" e_minalloc: %04X\n", dos_hdr->e_minalloc);
            printf(" e_maxalloc: %04X\n", dos_hdr->e_maxalloc);
            printf(" e_ss:       %04X\n", dos_hdr->e_ss);
            printf(" e_sp:       %04X\n", dos_hdr->e_sp);
            printf(" e_csum:     %04X\n", dos_hdr->e_csum);
            printf(" e_ip:       %04X\n", dos_hdr->e_ip);
            printf(" e_cs:       %04X\n", dos_hdr->e_cs);
            printf(" e_lfarlc:   %04X\n", dos_hdr->e_lfarlc);
            printf(" e_ovno:     %04X\n", dos_hdr->e_ovno);

            printf("\nEXE data is from offset %04X (%d) to %04X (%d).\n", exe_start, exe_start, exe_end, exe_end);
            goto cleanup;
        }
    }
    else
    {
        // a hack for raw COFF object files
        nt_hdr = (void *)(image - 4);
    }

    FAIL_IF(nt_hdr->FileHeader.Machine != IMAGE_FILE_MACHINE_I386, "Machine type not supported.\n");

    printf(" section     start       end    length     vaddr     vsize  flags   align\n");
    printf("-------------------------------------------------------------------------\n");

    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        const PIMAGE_SECTION_HEADER cur_sct = IMAGE_FIRST_SECTION(nt_hdr) + i;

        uint32_t align = (cur_sct->Characteristics & IMAGE_SCN_ALIGN_MASK)
                        ? (uint32_t)(1 << (((cur_sct->Characteristics & IMAGE_SCN_ALIGN_MASK) >> 20) - 1))
                        : nt_hdr->OptionalHeader.SectionAlignment;

        printf(
            "%8.8s %8"PRIX32"h %8"PRIX32"h %9"PRIu32" %8"PRIX32"h %9"PRIu32" %c%c%c%c%c%c %6"PRIX32"h\n",
            cur_sct->Name,
            cur_sct->PointerToRawData,
            cur_sct->PointerToRawData ? cur_sct->PointerToRawData + cur_sct->SizeOfRawData : 0,
            cur_sct->SizeOfRawData,
            cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase,
            cur_sct->Misc.VirtualSize,
            cur_sct->Characteristics & IMAGE_SCN_MEM_READ               ? 'r' : '-',
            cur_sct->Characteristics & IMAGE_SCN_MEM_WRITE              ? 'w' : '-',
            cur_sct->Characteristics & IMAGE_SCN_MEM_EXECUTE            ? 'x' : '-',
            cur_sct->Characteristics & IMAGE_SCN_CNT_CODE               ? 'c' : '-',
            cur_sct->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA   ? 'i' : '-',
            cur_sct->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA ? 'u' : '-',
            align
        );
    }

    printf("\n");

    char dirs[][40] = {
        "Export Directory",
        "Import Directory",
        "Resource Directory",
        "Exception Directory",
        "Security Directory",
        "Base Relocation Table",
        "Debug Directory",
        "Architecture Specific Data",
        "RVA of GP",
        "TLS Directory",
        "Load Configuration Directory",
        "Bound Import Directory in headers",
        "Import Address Table",
        "Delay Load Import Descriptors",
        "COM Runtime descriptor"
    };

    printf("DataDirectory                                   vaddr      size   section\n");
    printf("-------------------------------------------------------------------------\n");

    for (uint32_t i = 0; i < nt_hdr->OptionalHeader.NumberOfRvaAndSizes; i++)
    {
        if (!nt_hdr->OptionalHeader.DataDirectory[i].VirtualAddress)
            continue;

        char section[12] = { 0 };

        for (int x = 0; x < nt_hdr->FileHeader.NumberOfSections; x++)
        {
            const PIMAGE_SECTION_HEADER cur_sct = IMAGE_FIRST_SECTION(nt_hdr) + x;

            if (nt_hdr->OptionalHeader.DataDirectory[i].VirtualAddress >= cur_sct->VirtualAddress &&
                nt_hdr->OptionalHeader.DataDirectory[i].VirtualAddress < cur_sct->VirtualAddress + cur_sct->SizeOfRawData)
            {
                memcpy(section, (void*)cur_sct->Name, 8);
                break;
            }
        }

        printf(
            "%-43s %8"PRIX32"h %9"PRIu32"  %8.8s\n", 
            dirs[i],
            nt_hdr->OptionalHeader.DataDirectory[i].VirtualAddress + nt_hdr->OptionalHeader.ImageBase,
            nt_hdr->OptionalHeader.DataDirectory[i].Size,
            section);
    }

    printf("\n");

cleanup:
    if (image) free(image);
    if (fh)    fclose(fh);
    return ret;
}
