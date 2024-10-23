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

uint32_t offset_to_rva(uint32_t address, PIMAGE_NT_HEADERS nt_hdr)
{
    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER sct_hdr = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (address >= sct_hdr->PointerToRawData && address < sct_hdr->PointerToRawData + sct_hdr->SizeOfRawData)
        {
            return (address - sct_hdr->PointerToRawData) + sct_hdr->VirtualAddress;
        }
    }

    return address;
}

bool section_from_offset(uint32_t address, PIMAGE_NT_HEADERS nt_hdr, char* section, uint32_t size)
{
    if (size < 9)
        return false;

    memset(section, 0, size);

    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER sct_hdr = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (address >= sct_hdr->PointerToRawData && address < sct_hdr->PointerToRawData + sct_hdr->SizeOfRawData)
        {
            memcpy(section, (void*)sct_hdr->Name, 8);
            return true;
        }
    }

    return false;
}

int genpatch(int argc, char** argv)
{
    // decleration before more meaningful initialization for cleanup
    int ret = EXIT_SUCCESS;
    FILE* fh1 = NULL;
    uint8_t* image1 = NULL;
    FILE* fh2 = NULL;
    uint8_t* image2 = NULL;
    FILE* ofh = NULL;

    FAIL_IF(argc < 3, "usage: petool genpatch <image1> <image2> [ofile]\n");

    uint32_t length1;
    FAIL_IF_SILENT(open_and_read(&fh1, (int8_t**)&image1, &length1, argv[1], "rb"));

    PIMAGE_DOS_HEADER dos_hdr1 = (void *)image1;
    PIMAGE_NT_HEADERS nt_hdr1 = (void *)(image1 + dos_hdr1->e_lfanew);

    FAIL_IF(length1 < 512, "File1 too small.\n");
    FAIL_IF(dos_hdr1->e_magic != IMAGE_DOS_SIGNATURE, "File1 DOS signature invalid.\n");
    FAIL_IF(nt_hdr1->Signature != IMAGE_NT_SIGNATURE, "File1 NT signature invalid.\n");

    uint32_t length2;
    FAIL_IF_SILENT(open_and_read(&fh2, (int8_t**)&image2, &length2, argv[2], "rb"));

    PIMAGE_DOS_HEADER dos_hdr2 = (void*)image2;
    PIMAGE_NT_HEADERS nt_hdr2 = (void*)(image2 + dos_hdr2->e_lfanew);

    FAIL_IF(length2 < 512, "File2 too small.\n");
    FAIL_IF(dos_hdr2->e_magic != IMAGE_DOS_SIGNATURE, "File2 DOS signature invalid.\n");
    FAIL_IF(nt_hdr2->Signature != IMAGE_NT_SIGNATURE, "File2 NT signature invalid.\n");

    if (argc > 3)
    {
        ofh = fopen(argv[3], "w");
        FAIL_IF_PERROR(ofh == NULL, "%s");
    }
    else
    {
        static char path[MAX_PATH];
        FAIL_IF(_snprintf(path, sizeof(path) -1, "%s-patch.txt", argv[1]) < 0, "Fail - Path truncated\n");

        ofh = fopen(path, "w");
        FAIL_IF_PERROR(ofh == NULL, "%s");
    }

    uint32_t ignored = 0;

    fprintf(ofh, "Comparing files %s and %s\n\n", argv[1], argv[2]);

    for (uint32_t i = 0, len = 0; i < length1 && i < length2; i++)
    {
        if (image1[i] != image2[i])
        {
            if (len == 0)
            {
                char section[12] = { 0 };
                if (!section_from_offset(i, nt_hdr1, section, sizeof(section)))
                {
                    // Not within a section (fileheader/debuginfo/cert etc...) - ignored
                    ignored++;
                    continue;
                }

                fprintf(ofh, "%08X (%s):\n", i, section);
            }

            len++;
        }
        else if (len != 0)
        {
            fprintf(ofh, "    SETBYTES(0x%08X, \"", nt_hdr1->OptionalHeader.ImageBase + offset_to_rva(i - len, nt_hdr1));
            for (uint32_t x = 0; x < len; x++)
            {
                fprintf(ofh, "\\x%02X", image1[i - (len - x)]);
            }
            fprintf(ofh, "\");\n");

            fprintf(ofh, "    SETBYTES(0x%08X, \"", nt_hdr2->OptionalHeader.ImageBase + offset_to_rva(i - len, nt_hdr2));
            for (uint32_t x = 0; x < len; x++)
            {
                fprintf(ofh, "\\x%02X", image2[i - (len - x)]);
            }
            fprintf(ofh, "\");\n");

            len = 0;
        }
    }

    if (length1 != length2)
    {
        fprintf(ofh, "\nWARNING: file1 size does not match file2 size\n");
    }

    if (ignored > 0)
    {
        fprintf(ofh, "\nWARNING: %u bytes ignored (data not within a section)\n", ignored);
    }

cleanup:
    if (image1) free(image1);
    if (fh1) fclose(fh1);
    if (image2) free(image2);
    if (fh2) fclose(fh2);
    if (ofh) fclose(ofh);
    return ret;
}
