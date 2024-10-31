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

int export(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    int8_t *image = NULL;
    uint32_t length;

    FAIL_IF(argc < 2, "usage: petool export <image> [section]\n");
    FAIL_IF_SILENT(open_and_read(NULL, &image, &length, argv[1], NULL));
    FAIL_IF(!is_supported_pe_image(image, length), "File is not a valid i386 Portable Executable (PE) image.\n");

    PIMAGE_DOS_HEADER dos_hdr = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr = (void *)(image + dos_hdr->e_lfanew);

    char *section = argc > 2 ? (char *)argv[2] : ".data";
    int8_t *data = NULL;
    int32_t data_len = 0;

    for (int32_t i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER sct_hdr = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (strncmp(section, (char *)sct_hdr->Name, 8) == 0)
        {
            data = image + sct_hdr->PointerToRawData;
            data_len = sct_hdr->SizeOfRawData;
            break;
        }

        sct_hdr++;
    }

    FAIL_IF(data == NULL, "No '%s' section in given PE image.\n", section);

    fwrite(data, data_len, 1, stdout);

cleanup:
    if (image) free(image);
    return ret;
}
