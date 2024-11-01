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

int genpatch(int argc, char** argv)
{
    // decleration before more meaningful initialization for cleanup
    int ret = EXIT_SUCCESS;
    uint8_t* image1 = NULL;
    uint8_t* image2 = NULL;
    FILE* ofh1 = NULL;
    FILE* ofh2 = NULL;
    uint32_t length1;
    uint32_t length2;

    FAIL_IF(argc < 3, "usage: petool genpatch <image1> <image2> [ofile1] [ofile2]\n");
    FAIL_IF_SILENT(open_and_read(NULL, (int8_t**)&image1, &length1, argv[1], NULL));
    FAIL_IF(!is_supported_pe_image((int8_t*)image1, length1), "File1 is not a valid i386 Portable Executable (PE) image.\n");

    PIMAGE_DOS_HEADER dos_hdr1 = (void*)image1;
    PIMAGE_NT_HEADERS nt_hdr1 = (void*)(image1 + dos_hdr1->e_lfanew);

    FAIL_IF_SILENT(open_and_read(NULL, (int8_t**)&image2, &length2, argv[2], NULL));
    FAIL_IF(!is_supported_pe_image((int8_t*)image2, length2), "File2 is not a valid i386 Portable Executable (PE) image.\n");

    PIMAGE_DOS_HEADER dos_hdr2 = (void*)image2;
    PIMAGE_NT_HEADERS nt_hdr2 = (void*)(image2 + dos_hdr2->e_lfanew);

    if (argc > 4)
    {
        ofh1 = fopen(argv[3], "w");
        FAIL_IF_PERROR(ofh1 == NULL, "%s");

        ofh2 = fopen(argv[4], "w");
        FAIL_IF_PERROR(ofh2 == NULL, "%s");
    }
    else
    {
        static char path[MAX_PATH];
        FAIL_IF(snprintf(path, sizeof(path) - 1, "%s-patch.cpp", file_basename(argv[1])) < 0, "Fail - Path1 truncated\n");

        ofh1 = fopen(path, "w");
        FAIL_IF_PERROR(ofh1 == NULL, "%s");

        FAIL_IF(snprintf(path, sizeof(path) - 1, "%s-patch.cpp", file_basename(argv[2])) < 0, "Fail - Path2 truncated\n");

        ofh2 = fopen(path, "w");
        FAIL_IF_PERROR(ofh2 == NULL, "%s");
    }

    uint32_t dos_hdr_diff = 0;
    uint32_t file_hdr_diff = 0;
    uint32_t opt_hdr_diff = 0;
    uint32_t sct_hdr_diff = 0;
    uint32_t unknown_diff = 0;
    char section[12] = { 0 };

    fprintf(ofh1, "#include \"macros/patch.h\"\n\n");
    fprintf(ofh1, "//%s -> %s\n\n", argv[1], argv[2]);

    fprintf(ofh2, "#include \"macros/patch.h\"\n\n");
    fprintf(ofh2, "//%s -> %s\n\n", argv[2], argv[1]);

    for (uint32_t i = 0, len = 0; i < length1 && i < length2; i++)
    {
        if (image1[i] != image2[i])
        {
            if (len == 0)
            {
                if (!section_from_offset(i, nt_hdr1, section, sizeof(section)))
                {
                    // Not within a section (headers/debuginfo/cert etc...) - ignored

                    uint32_t file_hdr = dos_hdr1->e_lfanew + 4;
                    uint32_t opt_hdr = file_hdr + sizeof(IMAGE_FILE_HEADER);
                    uint32_t sct_hdr = opt_hdr + nt_hdr1->FileHeader.SizeOfOptionalHeader;

                    if (i < sizeof(IMAGE_DOS_HEADER))
                    {
                        dos_hdr_diff++;
                    } 
                    else if (i >= file_hdr && i < opt_hdr)
                    {
                        file_hdr_diff++;
                    }
                    else if (i >= opt_hdr && i < sct_hdr)
                    {
                        opt_hdr_diff++;
                    }
                    else if (
                        i >= sct_hdr && i < sct_hdr + (nt_hdr1->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER)))
                    {
                        sct_hdr_diff++;
                    }
                    else
                    {
                        unknown_diff++;
                    }

                    continue;
                }
            }

            len++;
        }
        else if (len != 0)
        {
            fprintf(ofh1, "SETBYTES(0x%08X, \"", nt_hdr1->OptionalHeader.ImageBase + offset_to_rva(i - len, nt_hdr1));
            for (uint32_t x = 0; x < len; x++)
            {
                fprintf(ofh1, "\\x%02X", image2[i - (len - x)]);
            }
            fprintf(ofh1, "\"); //%08X (%s)\n", i - len, section);

            fprintf(ofh2, "SETBYTES(0x%08X, \"", nt_hdr2->OptionalHeader.ImageBase + offset_to_rva(i - len, nt_hdr2));
            for (uint32_t x = 0; x < len; x++)
            {
                fprintf(ofh2, "\\x%02X", image1[i - (len - x)]);
            }
            fprintf(ofh2, "\"); //%08X (%s)\n", i - len, section);

            len = 0;
        }
    }

    if (length1 != length2)
    {
        char s[] = "\n//WARNING: file1 size does not match file2 size\n";
        fprintf(ofh1, "%s", s);
        fprintf(ofh2, "%s", s);
    }

    if (dos_hdr_diff > 0)
    {
        char s[] = "\n//WARNING: %u bytes changed in IMAGE_DOS_HEADER\n";
        fprintf(ofh1, s, dos_hdr_diff);
        fprintf(ofh2, s, dos_hdr_diff);
    }

    if (file_hdr_diff > 0)
    {
        char s[] = "\n//WARNING: %u bytes changed in IMAGE_FILE_HEADER\n";
        fprintf(ofh1, s, file_hdr_diff);
        fprintf(ofh2, s, file_hdr_diff);
    }

    if (opt_hdr_diff > 0)
    {
        char s[] = "\n//WARNING: %u bytes changed in IMAGE_OPTIONAL_HEADER\n";
        fprintf(ofh1, s, opt_hdr_diff);
        fprintf(ofh2, s, opt_hdr_diff);
    }

    if (sct_hdr_diff > 0)
    {
        char s[] = "\n//WARNING: %u bytes changed in IMAGE_SECTION_HEADER[]\n";
        fprintf(ofh1, s, sct_hdr_diff);
        fprintf(ofh2, s, sct_hdr_diff);
    }

    if (unknown_diff > 0)
    {
        char s[] = "\n//WARNING: %u unknown bytes changed (data not within any section or headers)\n";
        fprintf(ofh1, s, unknown_diff);
        fprintf(ofh2, s, unknown_diff);
    }

cleanup:
    if (image1) free(image1);
    if (image2) free(image2);
    if (ofh1) fclose(ofh1);
    if (ofh2) fclose(ofh2);
    return ret;
}
