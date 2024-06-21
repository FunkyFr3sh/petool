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

extern bool g_sym_imports_enabled;

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

    fprintf(ofh, "LDFLAGS     =");

    if (nt_hdr->OptionalHeader.SectionAlignment != 0x1000)
        fprintf(ofh, " -Wl,--section-alignment=0x%"PRIX32"", nt_hdr->OptionalHeader.SectionAlignment);

    //if (nt_hdr->OptionalHeader.FileAlignment != 0x200)
        fprintf(ofh, " -Wl,--file-alignment=0x%"PRIX32"", nt_hdr->OptionalHeader.FileAlignment);

    if (nt_hdr->OptionalHeader.ImageBase != 0x00400000)
        fprintf(ofh, " -Wl,--image-base=0x%08"PRIX32"", nt_hdr->OptionalHeader.ImageBase);

    if (nt_hdr->OptionalHeader.Subsystem == 2)
        fprintf(ofh, " -Wl,--subsystem=windows");

    if (!(nt_hdr->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NX_COMPAT))
        fprintf(ofh, " -Wl,--disable-nxcompat");

    fprintf(ofh, " -Wl,--disable-reloc-section -Wl,--enable-stdcall-fixup -static");

    if (!g_sym_imports_enabled)
        fprintf(ofh, " -nostdlib");

    fprintf(ofh, "\n");

    fprintf(ofh, "ASFLAGS     = -Iinc\n");
    fprintf(ofh, "NFLAGS      = -Iinc -f elf\n");
    fprintf(ofh, "CFLAGS      = -Iinc -O2 -march=pentium4 -Wall -masm=intel\n");
    fprintf(ofh, "CXXFLAGS    = -Iinc -O2 -march=pentium4 -Wall -masm=intel\n");

    fprintf(ofh, "\n");

    if (g_sym_imports_enabled)
    {
    fprintf(ofh, "LIBS        = -lgdi32\n");

    fprintf(ofh, "\n");
    }

    fprintf(ofh, "OBJS        =");

    if (strcmp(argv[0], "genmak") != 0)
    {
        fprintf(ofh, " \\\n				sym.o");
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 2 && nt_hdr->OptionalHeader.DataDirectory[2].VirtualAddress)
    {
        fprintf(ofh, " \\\n				rsrc.o");
    }

    if (strcmp(argv[0], "genmak") != 0)
    {
        fprintf(ofh, " \\\n				src/imports.o");
        fprintf(ofh, " \\\n				src/start.o");
    }

    fprintf(ofh, "\n\n");

    fprintf(ofh, "CC          = i686-w64-mingw32-gcc\n");
    fprintf(ofh, "CXX         = i686-w64-mingw32-g++\n");
    fprintf(ofh, "AS          = i686-w64-mingw32-as\n");
    fprintf(ofh, "STRIP      ?= i686-w64-mingw32-strip\n");
    fprintf(ofh, "WINDRES    ?= i686-w64-mingw32-windres\n");
    fprintf(ofh, "PETOOL     ?= petool\n");
    fprintf(ofh, "NASM       ?= nasm\n");

    fprintf(ofh, "\n");

    /* Make sure the DataDirectory VA is within a section (Heroes of Might and Magic 3 / LoadConfig) */
    uint32_t code_start = UINT32_MAX;

    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        const PIMAGE_SECTION_HEADER cur_sct = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (cur_sct->VirtualAddress && cur_sct->VirtualAddress < code_start)
        {
            code_start = cur_sct->VirtualAddress;
        }
    }

    if (code_start == UINT32_MAX)
    {
        code_start = nt_hdr->OptionalHeader.SectionAlignment;
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 0 && 
        nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress && 
        nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress >= code_start)
    {
        fprintf(
            ofh, 
            "EXPORTDIR   =  0 0x%"PRIX32" %"PRIu32"\n",
            nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress, 
            nt_hdr->OptionalHeader.DataDirectory[0].Size);
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 1 &&
        nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress >= code_start)
    {
        fprintf(
            ofh,
            "IMPORTDIR   =  1 0x%"PRIX32" %"PRIu32"\n",
            nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress,
            nt_hdr->OptionalHeader.DataDirectory[1].Size);
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 3 && 
        nt_hdr->OptionalHeader.DataDirectory[3].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[3].VirtualAddress >= code_start)
    {
        fprintf(
            ofh,
            "EXCEPTDIR   =  3 0x%"PRIX32" %"PRIu32"\n",
            nt_hdr->OptionalHeader.DataDirectory[3].VirtualAddress,
            nt_hdr->OptionalHeader.DataDirectory[3].Size);
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 7 && 
        nt_hdr->OptionalHeader.DataDirectory[7].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[7].VirtualAddress >= code_start)
    {
        fprintf(
            ofh,
            "ARCHITECTUR =  7 0x%"PRIX32" %"PRIu32"\n",
            nt_hdr->OptionalHeader.DataDirectory[7].VirtualAddress,
            nt_hdr->OptionalHeader.DataDirectory[7].Size);
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 8 && 
        nt_hdr->OptionalHeader.DataDirectory[8].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[8].VirtualAddress >= code_start)
    {
        fprintf(
            ofh,
            "GLOBALPTR   =  8 0x%"PRIX32" %"PRIu32"\n",
            nt_hdr->OptionalHeader.DataDirectory[8].VirtualAddress,
            nt_hdr->OptionalHeader.DataDirectory[8].Size);
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 9 &&
        nt_hdr->OptionalHeader.DataDirectory[9].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[9].VirtualAddress >= code_start)
    {
        fprintf(
            ofh,
            "TLSDIR      =  9 0x%"PRIX32" %"PRIu32"\n",
            nt_hdr->OptionalHeader.DataDirectory[9].VirtualAddress,
            nt_hdr->OptionalHeader.DataDirectory[9].Size);
    }
    else
    {
        fprintf(ofh, "TLSDIR      =  9 0x0 0\n");
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 10 && 
        nt_hdr->OptionalHeader.DataDirectory[10].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[10].VirtualAddress >= code_start)
    {
        fprintf(
            ofh, 
            "LOADCFGDIR  = 10 0x%"PRIX32" %"PRIu32"\n",
            nt_hdr->OptionalHeader.DataDirectory[10].VirtualAddress, 
            nt_hdr->OptionalHeader.DataDirectory[10].Size);
    }
    
    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 12 &&
        nt_hdr->OptionalHeader.DataDirectory[12].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[12].Size &&
        nt_hdr->OptionalHeader.DataDirectory[12].VirtualAddress >= code_start)
    {
        fprintf(
            ofh, 
            "IAT         = 12 0x%"PRIX32" %"PRIu32"\n", 
            nt_hdr->OptionalHeader.DataDirectory[12].VirtualAddress, 
            nt_hdr->OptionalHeader.DataDirectory[12].Size);
    }
    else
    {
        fprintf(ofh, "IAT         = 12 0x0 0\n");
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 13 && 
        nt_hdr->OptionalHeader.DataDirectory[13].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[13].VirtualAddress >= code_start)
    {
        fprintf(
            ofh,
            "DIMPORTDESC = 13 0x%"PRIX32" %"PRIu32"\n",
            nt_hdr->OptionalHeader.DataDirectory[13].VirtualAddress,
            nt_hdr->OptionalHeader.DataDirectory[13].Size);
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 14 && 
        nt_hdr->OptionalHeader.DataDirectory[14].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[14].VirtualAddress >= code_start)
    {
        fprintf(
            ofh,
            "COMRUNDESC  = 14 0x%"PRIX32" %"PRIu32"\n",
            nt_hdr->OptionalHeader.DataDirectory[14].VirtualAddress,
            nt_hdr->OptionalHeader.DataDirectory[14].Size);
    }

    fprintf(ofh, "\n");

    fprintf(ofh, "all: $(OUTPUT)\n\n");

    fprintf(ofh, "%%.o: %%.asm\n");
    fprintf(ofh, "	$(NASM) $(NFLAGS) -o $@ $<\n\n");

    fprintf(ofh, "%%.o: %%.rc\n");
    fprintf(ofh, "	$(WINDRES) $(WFLAGS) $< $@\n\n");

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 2 && nt_hdr->OptionalHeader.DataDirectory[2].VirtualAddress)
    {
        fprintf(ofh, "rsrc.o: $(INPUT)\n");
        fprintf(ofh, "	$(PETOOL) re2obj $< $@\n\n");
    }

    fprintf(ofh, "$(OUTPUT): $(OBJS)\n");
    fprintf(ofh, "	$(CXX) $(LDFLAGS) -T $(LDS) -o \"$@\" $^ $(LIBS)\n");

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 0 && 
        nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[0].VirtualAddress >= code_start)
    {
        fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(EXPORTDIR) || ($(RM) \"$@\" && exit 1)\n");
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 0 &&
        nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress >= code_start)
    {
        fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(IMPORTDIR) || ($(RM) \"$@\" && exit 1)\n");
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 3 && 
        nt_hdr->OptionalHeader.DataDirectory[3].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[3].VirtualAddress >= code_start)
    {
        fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(EXCEPTDIR) || ($(RM) \"$@\" && exit 1)\n");
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 7 && 
        nt_hdr->OptionalHeader.DataDirectory[7].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[7].VirtualAddress >= code_start)
    {
        fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(ARCHITECTUR) || ($(RM) \"$@\" && exit 1)\n");
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 8 && 
        nt_hdr->OptionalHeader.DataDirectory[8].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[8].VirtualAddress >= code_start)
    {
        fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(GLOBALPTR) || ($(RM) \"$@\" && exit 1)\n");
    }

    fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(TLSDIR) || ($(RM) \"$@\" && exit 1)\n");

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 10 && 
        nt_hdr->OptionalHeader.DataDirectory[10].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[10].VirtualAddress >= code_start)
    {
        fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(LOADCFGDIR) || ($(RM) \"$@\" && exit 1)\n");
    }

    fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(IAT) || ($(RM) \"$@\" && exit 1)\n");

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 13 && 
        nt_hdr->OptionalHeader.DataDirectory[13].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[13].VirtualAddress >= code_start)
    {
        fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(DIMPORTDESC) || ($(RM) \"$@\" && exit 1)\n");
    }

    if (nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 14 && 
        nt_hdr->OptionalHeader.DataDirectory[14].VirtualAddress &&
        nt_hdr->OptionalHeader.DataDirectory[14].VirtualAddress >= code_start)
    {
        fprintf(ofh, "	$(PETOOL) setdd \"$@\" $(COMRUNDESC) || ($(RM) \"$@\" && exit 1)\n");
    }

    fprintf(ofh, "	$(PETOOL) setsc \"$@\" .p_text 0x60000020 || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "	$(PETOOL) patch \"$@\" || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "	$(STRIP) -R .patch \"$@\" || ($(RM) \"$@\" && exit 1)\n");
    fprintf(ofh, "	$(PETOOL) dump \"$(INPUT)\"\n");
    fprintf(ofh, "	$(PETOOL) dump \"$@\"\n");

    fprintf(ofh, "\n");

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
