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

int setsc(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    FILE   *fh    = NULL;
    int8_t *image = NULL;
    uint32_t length;

    FAIL_IF(argc != 4, "usage: petool setsc <image> <section> <Characteristics>\n");
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "r+b"));
    FAIL_IF(!is_supported_pe_image(image, length), "File is not a valid i386 Portable Executable (PE) image.\n");

    PIMAGE_DOS_HEADER dos_hdr = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr  = (void *)(image + dos_hdr->e_lfanew);

    uint32_t flags = strtoul(argv[3], NULL, 0);
    FAIL_IF(flags == 0, "Characteristics can't be zero.\n");

    for (int32_t i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER sct_hdr = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (strncmp(argv[2], (char *)sct_hdr->Name, 8) == 0)
        {
            sct_hdr->Characteristics = flags;    // update characteristics

            nt_hdr->OptionalHeader.CheckSum = 0; // FIXME: implement checksum calculation
            rewind(fh);                          // write to file
            FAIL_IF_PERROR(fwrite(image, length, 1, fh) != 1, "Error writing executable");
            goto cleanup;                        // done
        }
    }

    fprintf(stderr, "No '%s' section in given PE image.\n", argv[2]);

cleanup:
    if (image) free(image);
    if (fh)    fclose(fh);
    return ret;
}
