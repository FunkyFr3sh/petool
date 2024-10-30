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

int gensym(int argc, char** argv)
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

    FAIL_IF(length < 512, "File too small.\n");
    FAIL_IF(dos_hdr->e_magic != IMAGE_DOS_SIGNATURE, "File DOS signature invalid.\n");
    FAIL_IF(nt_hdr->Signature != IMAGE_NT_SIGNATURE, "File NT signature invalid.\n");
    FAIL_IF(nt_hdr->FileHeader.Machine != IMAGE_FILE_MACHINE_I386, "Machine type is not i386.\n");

    bool is_clr = nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 14 && nt_hdr->OptionalHeader.DataDirectory[14].VirtualAddress;
    FAIL_IF(is_clr, ".NET assembly not supported\n");

    fprintf(ofh, "#include \"macros/setsym.h\"\n\n\n");
    fprintf(ofh, "/* vars */\n\n\n");
    fprintf(ofh, "/* functions */\n\n");
    fprintf(ofh, "SETCGLOB(0x%08"PRIX32", OriginalCRTStartup);\n", (nt_hdr->OptionalHeader.ImageBase + nt_hdr->OptionalHeader.AddressOfEntryPoint));
    
    if (strcmp(argv[0], "gensym") == 0 && 
        nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 1 && 
        nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[1].Size)
    {
        fprintf(ofh, "/* imports */\n\n");

        uint32_t offset = rva_to_offset(nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress, nt_hdr);
        IMAGE_IMPORT_DESCRIPTOR* i = (void*)(image + offset);

        while (i->FirstThunk) {
            char name[260] = { 0 };

            if (i->Name != 0) {
                strncpy(
                    name,
                    (char*)(image + rva_to_offset(i->Name, nt_hdr)),
                    sizeof name - 1
                );
                
                fprintf(ofh, "\n/* %s */\n", name);
            }

            uint32_t thunk = i->OriginalFirstThunk ? i->OriginalFirstThunk : i->FirstThunk;

            PIMAGE_THUNK_DATA32 ft =
                (PIMAGE_THUNK_DATA32)(image + rva_to_offset(thunk, nt_hdr));

            uint32_t function = nt_hdr->OptionalHeader.ImageBase + i->FirstThunk;

            while (ft->u1.AddressOfData)
            {
                PIMAGE_IMPORT_BY_NAME import =
                    (PIMAGE_IMPORT_BY_NAME)(image + rva_to_offset(ft->u1.AddressOfData, nt_hdr));

                if ((ft->u1.Ordinal & IMAGE_ORDINAL_FLAG32) == 0)
                {
                    fprintf(ofh, "SETCGLOB(0x%08"PRIX32", _imp__%s);\n", function, (const char*)import->Name);
                }
                else
                {
                    char* p = strrchr(name, '.');
                    if (p)
                    {
                        *p = '\0';
                    }

                    int ordinal = (ft->u1.Ordinal & ~IMAGE_ORDINAL_FLAG32) & 0xffff;

                    fprintf(ofh, "SETCGLOB(0x%08"PRIX32", _imp__%s_Ordinal_%d);\n", function, name, ordinal);
                }


                ft++;
                function += sizeof(IMAGE_THUNK_DATA32);
            }

            i++;
        }
    }

cleanup:
    if (image) free(image);
    if (argc > 2)
    {
        if (ofh)   fclose(ofh);
    }
    return ret;
}
