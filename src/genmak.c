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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "pe.h"
#include "cleanup.h"
#include "common.h"

uint32_t rva_to_offset(uint32_t address, PIMAGE_NT_HEADERS nt_hdr);

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

    strncpy(base, file_basename(argv[1]), sizeof(base) - 1);
    char *p = strrchr(base, '.');
    if (p)
    {
        *p = '\0';
    }

    fprintf(ofh, "-include config.mk\n\n");
    fprintf(ofh, "INPUT       = %s.dat\n", base);
    fprintf(ofh, "OUTPUT      = %s\n", file_escaped_basename(argv[1]));
    fprintf(ofh, "LDS         = %s.lds\n", base);

    fprintf(ofh, "\n");

    if (nt_hdr->OptionalHeader.DataDirectory[10].VirtualAddress)
    {
        fprintf(ofh, "LOADCONFIG  = 0x%"PRIX32" %d\n", nt_hdr->OptionalHeader.DataDirectory[10].VirtualAddress, nt_hdr->OptionalHeader.DataDirectory[10].Size);
    }

    if (nt_hdr->OptionalHeader.DataDirectory[9].VirtualAddress)
    {
        fprintf(ofh, "TLS         = 0x%"PRIX32" %d\n", nt_hdr->OptionalHeader.DataDirectory[9].VirtualAddress, nt_hdr->OptionalHeader.DataDirectory[9].Size);
    }

    if (nt_hdr->OptionalHeader.DataDirectory[12].VirtualAddress && nt_hdr->OptionalHeader.DataDirectory[12].Size)
    {
        fprintf(ofh, "IAT         = 0x%"PRIX32" %d\n", nt_hdr->OptionalHeader.DataDirectory[12].VirtualAddress, nt_hdr->OptionalHeader.DataDirectory[12].Size);
    }
    else if (nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress && nt_hdr->OptionalHeader.DataDirectory[1].Size)
    {
        /* IAT must be set or PE loader will fail to initialize the imports when they're in a read-only section */
        uint32_t offset = rva_to_offset(nt_hdr->OptionalHeader.ImageBase + nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress, nt_hdr);
        IMAGE_IMPORT_DESCRIPTOR* i = (void*)(image + offset);

        uint32_t iat_start = UINT32_MAX;
        uint32_t iat_end = 0;

        while (i->FirstThunk)
        {
            iat_start = i->FirstThunk < iat_start ? i->FirstThunk : iat_start;
            iat_end = i->FirstThunk > iat_end ? i->FirstThunk : iat_end;
            i++;
        }

        if (iat_end)
        {
            PIMAGE_THUNK_DATA32 ft =
                (PIMAGE_THUNK_DATA32)(image + rva_to_offset(nt_hdr->OptionalHeader.ImageBase + iat_end, nt_hdr));

            while (ft->u1.Function)
            {
                ft++;
                iat_end += sizeof(IMAGE_THUNK_DATA32);
            }

            fprintf(ofh, "IAT         = 0x%"PRIX32" %d\n", iat_start, iat_end - iat_start + 4);
        }
    }

    fprintf(ofh, "\n");

    fprintf(ofh, "LDFLAGS     =");

    if (nt_hdr->OptionalHeader.SectionAlignment != 0x1000)
        fprintf(ofh, " -Wl,--section-alignment=0x%"PRIX32"", nt_hdr->OptionalHeader.SectionAlignment);

    if (nt_hdr->OptionalHeader.ImageBase != 0x00400000)
        fprintf(ofh, " -Wl,--image-base=0x%08"PRIX32"", nt_hdr->OptionalHeader.ImageBase);

    if (nt_hdr->OptionalHeader.Subsystem == 2)
        fprintf(ofh, " -Wl,--subsystem=windows");

    if (!(nt_hdr->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NX_COMPAT))
        fprintf(ofh, " -Wl,--disable-nxcompat");

    fprintf(ofh, " -Wl,--disable-reloc-section -Wl,--enable-stdcall-fixup -static");

    fprintf(ofh, "\n");

    fprintf(ofh, "ASFLAGS     = -Iinc\n");
    fprintf(ofh, "NFLAGS      = -Iinc -f elf\n");
    fprintf(ofh, "CFLAGS      = -Iinc -O2 -march=pentium4 -Wall -masm=intel\n");
    fprintf(ofh, "CXXFLAGS    = -Iinc -O2 -march=pentium4 -Wall -masm=intel\n");

    fprintf(ofh, "\n");

    fprintf(ofh, "LIBS        = -lgdi32\n");

    fprintf(ofh, "\n");

    fprintf(ofh, "OBJS        =");

    if (strcmp(argv[0], "genmak") != 0)
    {
        fprintf(ofh, " \\\n				sym.o");
    }

    if (nt_hdr->OptionalHeader.DataDirectory[2].VirtualAddress)
    {
        fprintf(ofh, " \\\n				rsrc.o");
    }

    if (strcmp(argv[0], "genmak") != 0)
    {
        fprintf(ofh, " \\\n				src/winmain.o");
    }

    fprintf(ofh, "\n\n");

    fprintf(ofh, "CC          = i686-w64-mingw32-gcc\n");
    fprintf(ofh, "CXX         = i686-w64-mingw32-g++\n");
    fprintf(ofh, "STRIP      ?= i686-w64-mingw32-strip\n");
    fprintf(ofh, "WINDRES    ?= i686-w64-mingw32-windres\n");
    fprintf(ofh, "PETOOL     ?= petool\n");
    fprintf(ofh, "NASM       ?= nasm\n");

    fprintf(ofh, "\n");

    fprintf(ofh, "all: $(OUTPUT)\n\n");

    fprintf(ofh, "%%.o: %%.asm\n");
    fprintf(ofh, "	$(NASM) $(NFLAGS) -o $@ $<\n\n");

    fprintf(ofh, "%%.o: %%.c\n");
    fprintf(ofh, "	$(CC) $(CFLAGS) -c -o $@ $<\n\n");

    fprintf(ofh, "%%.o: %%.cpp\n");
    fprintf(ofh, "	$(CXX) $(CXXFLAGS) -c -o $@ $<\n\n");

    fprintf(ofh, "%%.o: %%.s\n");
    fprintf(ofh, "	$(AS) $(ASFLAGS) -o $@ $<\n\n");

    fprintf(ofh, "%%.o: %%.rc\n");
    fprintf(ofh, "	$(WINDRES) $(WINDRES_FLAGS) $< $@\n\n");

    if (nt_hdr->OptionalHeader.DataDirectory[2].VirtualAddress)
    {
        fprintf(ofh, "rsrc.o: $(INPUT)\n");
        fprintf(ofh, "	$(PETOOL) re2obj $(INPUT) $@\n\n");
    }

    fprintf(ofh, "$(OUTPUT): $(LDS) $(INPUT) $(OBJS)\n");
    fprintf(ofh, "	$(CXX) $(LDFLAGS) -T $(LDS) -o \"$@\" $(OBJS) $(LIBS)\n");
    fprintf(ofh, "ifneq (,$(LOADCONFIG))\n");
    fprintf(ofh, "	$(PETOOL) setdd \"$@\" 10 $(LOADCONFIG) || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "endif\n");
    fprintf(ofh, "ifneq (,$(TLS))\n");
    fprintf(ofh, "	$(PETOOL) setdd \"$@\" 9 $(TLS) || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "endif\n");
    fprintf(ofh, "ifneq (,$(IAT))\n");
    fprintf(ofh, "	$(PETOOL) setdd \"$@\" 12 $(IAT) || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "endif\n");
    fprintf(ofh, "	$(PETOOL) setc  \"$@\" .p_text 0x60000020 || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "	$(PETOOL) patch \"$@\" || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "	$(STRIP) -R .patch \"$@\" || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "	$(PETOOL) dump \"$@\"\n\n");

    fprintf(ofh, "clean:\n");
    fprintf(ofh, "	$(RM) $(OUTPUT) $(OBJS)\n");

cleanup:
    if (argc > 2)
    {
        if (ofh)   fclose(ofh);
    }

    if (image) free(image);
    if (fh)    fclose(fh);
    return ret;
}
