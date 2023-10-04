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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "pe.h"
#include "cleanup.h"
#include "common.h"

int genmak(int argc, char **argv)
{
    int     ret   = EXIT_SUCCESS;
    FILE   *fh    = NULL;
    int8_t *image = NULL;
    FILE   *ofh   = stdout;
    char   base[256] = { '\0' };

    FAIL_IF(argc < 2, "usage: petool genmak <image> [ofile]\n");

    if (argc > 2)
    {
        FAIL_IF(file_exists(argv[2]), "%s: output file already exists.\n", argv[2]);
        ofh = fopen(argv[2], "w");
        FAIL_IF_PERROR(ofh == NULL, "%s");
    }

    uint32_t length;
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "rb"));

    fclose(fh);
    fh = NULL; // for cleanup

    PIMAGE_DOS_HEADER dos_hdr = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr = (void *)(image + dos_hdr->e_lfanew);

    FAIL_IF(length < 512,                            "File too small.\n");
    FAIL_IF(dos_hdr->e_magic != IMAGE_DOS_SIGNATURE, "File DOS signature invalid.\n");
    FAIL_IF(nt_hdr->Signature != IMAGE_NT_SIGNATURE, "File NT signature invalid.\n");

    strcpy(base, file_basename(argv[1]));
    char *p = strrchr(base, '.');
    if (p)
    {
        *p = '\0';
    }

    fprintf(ofh, "-include config.mk\n\n");
    fprintf(ofh, "INPUT       = %s\n", file_basename(argv[1]));
    fprintf(ofh, "OUTPUT      = %sp.exe\n", base);
    fprintf(ofh, "LDS         = %sp.lds\n", base);

    fprintf(ofh, "IMPORTS     =");
    if (nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress)
    {
        fprintf(ofh, " 0x%"PRIX32" %d", nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress, nt_hdr->OptionalHeader.DataDirectory[1].Size);
    }
    fprintf(ofh, "\n");

    fprintf(ofh, "LDFLAGS     = --section-alignment=0x%"PRIX32, nt_hdr->OptionalHeader.SectionAlignment);

    if (nt_hdr->OptionalHeader.Subsystem == 2)
        fprintf(ofh, " --subsystem=windows");

    fprintf(ofh, " --enable-stdcall-fixup");

    fprintf(ofh, "\n");

    fprintf(ofh, "NFLAGS      = -f elf -Iinc/\n");
    fprintf(ofh, "CFLAGS      = -std=c99 -Iinc/ -O2 -march=i486\n");

    fprintf(ofh, "\n\n");

    fprintf(ofh, "OBJS        = ");
    if (nt_hdr->OptionalHeader.DataDirectory[2].VirtualAddress)
    {
        fprintf(ofh, "rsrc.o");
    }
    fprintf(ofh, "\n\n");

    fprintf(ofh, "PETOOL     ?= petool\n");
    fprintf(ofh, "STRIP      ?= strip\n");
    fprintf(ofh, "NASM       ?= nasm\n");
    fprintf(ofh, "WINDRES    ?= windres\n\n");

    fprintf(ofh, "all: $(OUTPUT)\n\n");

    fprintf(ofh, "%%.o: %%.asm\n");
    fprintf(ofh, "	$(NASM) $(NFLAGS) -o $@ $<\n\n");

    if (nt_hdr->OptionalHeader.DataDirectory[2].VirtualAddress)
    {
        fprintf(ofh, "rsrc.o: $(INPUT)\n");
        fprintf(ofh, "\t$(PETOOL) re2obj $(INPUT) $@\n\n");
    }

    fprintf(ofh, "$(OUTPUT): $(LDS) $(INPUT) $(OBJS)\n");
    fprintf(ofh, "\t$(LD) $(LDFLAGS) -T $(LDS) -o \"$@\" $(OBJS)\n");
    fprintf(ofh, "ifneq (,$(IMPORTS))\n");
    fprintf(ofh, "\t$(PETOOL) setdd \"$@\" 1 $(IMPORTS) || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "endif\n");
    fprintf(ofh, "\t$(PETOOL) patch \"$@\" || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "\t$(STRIP) -R .patch \"$@\" || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "\t$(PETOOL) dump \"$@\"\n\n");

    fprintf(ofh, "clean:\n");
    fprintf(ofh, "\t$(RM) $(OUTPUT) $(OBJS)\n");

cleanup:
    if (argc > 2)
    {
        if (ofh)   fclose(ofh);
    }

    if (image) free(image);
    if (fh)    fclose(fh);
    return ret;
}
