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
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "pe.h"
#include "cleanup.h"
#include "common.h"

int pe2obj(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    FILE   *ofh   = NULL;
    int8_t *image = NULL;
    uint32_t length;

    FAIL_IF(argc != 3, "usage: petool pe2obj <in> <out>\n");
    FAIL_IF_SILENT(open_and_read(NULL, &image, &length, argv[1], NULL));
    FAIL_IF(!is_supported_pe_image(image, length), "File is not a valid i386 Portable Executable (PE) image.\n");

    PIMAGE_DOS_HEADER dos_hdr = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr = (void *)(image + dos_hdr->e_lfanew);

    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        const PIMAGE_SECTION_HEADER cur_sct = IMAGE_FIRST_SECTION(nt_hdr) + i;
        if (cur_sct->PointerToRawData)
        {
            cur_sct->PointerToRawData -= dos_hdr->e_lfanew + 4;
        }

        if (cur_sct->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA && !(cur_sct->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA))
        {
            cur_sct->PointerToRawData = 0;
            cur_sct->SizeOfRawData = 0;
        }
    }

    ofh = fopen(argv[2], "wb");
    FAIL_IF_PERROR(!ofh, "Error opening output file");

    FAIL_IF_PERROR(fwrite((char *)image + (dos_hdr->e_lfanew + 4),
                         length - (dos_hdr->e_lfanew + 4),
                         1,
                         ofh) != 1,
                  "Failed to write object file to output file\n");

cleanup:
    if (image) free(image);
    if (ofh)   fclose(ofh);
    return ret;
}
