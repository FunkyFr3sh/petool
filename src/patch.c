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
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>

#include "pe.h"
#include "cleanup.h"
#include "common.h"

static struct {
    uint32_t start;
    uint32_t end;
} g_patch_offsets[4096];

int patch_image(int8_t *image, uint32_t address, int8_t *patch, uint32_t length)
{
    PIMAGE_DOS_HEADER dos_hdr       = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr        = (PIMAGE_NT_HEADERS)(image + dos_hdr->e_lfanew);

    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER sct_hdr = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (sct_hdr->VirtualAddress + nt_hdr->OptionalHeader.ImageBase <= address && address < sct_hdr->VirtualAddress + nt_hdr->OptionalHeader.ImageBase + sct_hdr->SizeOfRawData)
        {
            uint32_t offset = sct_hdr->PointerToRawData + (address - (sct_hdr->VirtualAddress + nt_hdr->OptionalHeader.ImageBase));

            if (sct_hdr->SizeOfRawData < length || sct_hdr->PointerToRawData + sct_hdr->SizeOfRawData < offset + length)
            {
                fprintf(stderr, "Error: Patch '%08"PRIX32"' is too long (%"PRIu32" bytes)\n", address, length);

                return EXIT_FAILURE;
            }

            memcpy(image + offset, patch, length);

            for (uint32_t i = 0; i < sizeof(g_patch_offsets) / sizeof(g_patch_offsets[0]); i++)
            {
                if (g_patch_offsets[i].start)
                {
                    if ((address >= g_patch_offsets[i].start && address < g_patch_offsets[i].end) ||
                        (address < g_patch_offsets[i].start && address + length - 1 >= g_patch_offsets[i].start))
                    {
                        fprintf(
                            stderr,
                            "Warning: Conflicting patches detected\n"
                            "    Patch 1 start = %08"PRIX32", end = %08"PRIX32", size = %"PRIu32" bytes\n"
                            "    Patch 2 start = %08"PRIX32", end = %08"PRIX32", size = %"PRIu32" bytes\n\n",
                            address,
                            address + length,
                            length,
                            g_patch_offsets[i].start,
                            g_patch_offsets[i].end,
                            g_patch_offsets[i].end - g_patch_offsets[i].start);
                    }
                }
                else
                {
                    g_patch_offsets[i].start = address;
                    g_patch_offsets[i].end = address + length;
                    break;
                }
            }

            //printf("PATCH  %8"PRId32" bytes -> %8"PRIX32"\n", length, address);
            return EXIT_SUCCESS;
        }
    }

    fprintf(stderr, "Error: memory address %08"PRIX32" not found in image\n", address);
    return EXIT_FAILURE;
}

uint32_t get_uint32(int8_t * *p)
{
    uint32_t ret = *(uint32_t *)*p;
    *p += sizeof(uint32_t);
    return ret;
}

int patch(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_FAILURE;
    FILE   *fh    = NULL;
    int8_t *image = NULL;
    uint32_t length;

    FAIL_IF(argc < 2, "usage: petool patch <image> [section]\n");
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "r+b"));
    FAIL_IF(!is_supported_pe_image(image, length), "File is not a valid i386 Portable Executable (PE) image.\n");

    PIMAGE_DOS_HEADER dos_hdr = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr = (void *)(image + dos_hdr->e_lfanew);

    char *section = argc > 2 ? (char *)argv[2] : ".patch";
    int8_t *patch = NULL;
    int32_t patch_len = 0;

    for (int32_t i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER sct_hdr = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (strncmp(section, (char *)sct_hdr->Name, 8) == 0)
        {
            patch = image + sct_hdr->PointerToRawData;
            patch_len = sct_hdr->Misc.VirtualSize;
        }
        else
        {
            //hack for bug in binutils - make sure vsize is not less than rawsize so we don't lose any data on strip
            if (sct_hdr->Misc.VirtualSize && sct_hdr->SizeOfRawData > sct_hdr->Misc.VirtualSize)
            {
                sct_hdr->Misc.VirtualSize = sct_hdr->SizeOfRawData;
            }
        }
    }

    if (patch == NULL)
    {
        fprintf(stderr, "Warning: No '%s' section in given PE image.\n", section);
        ret = EXIT_SUCCESS;
        goto cleanup;
    }

    uint32_t patch_count = 0;
    uint32_t patch_bytes = 0;

    for (int8_t *p = patch; p < patch + patch_len;)
    {
        uint32_t paddress = get_uint32(&p);
        if (paddress == 0)
        {
            fprintf(stderr, "Warning: Trailing zero address in '%s' section.\n", section);
            break;
        }

        uint32_t plength = get_uint32(&p);

        if (plength > 0)
        {
            FAIL_IF_SILENT(patch_image(image, paddress, p, plength) == EXIT_FAILURE);

            patch_count++;
            patch_bytes += plength;

            p += plength;
        }
        else
        {
            fprintf(
                stderr, 
                "Warning: Empty patch detected\n"
                "    Patch start = %08"PRIX32", size = 0 bytes\n\n", 
                paddress);
        }
    }

    fprintf(stderr, "Applied %"PRIu32" patches (%"PRIu32" bytes)\n", patch_count, patch_bytes);

    /* FIXME: implement checksum calculation */
    nt_hdr->OptionalHeader.CheckSum = 0;

    rewind(fh);
    FAIL_IF_PERROR(fwrite(image, length, 1, fh) != 1, "Error writing executable");

    ret = EXIT_SUCCESS;
cleanup:
    if (image) free(image);
    if (fh)    fclose(fh);
    return ret;
}
