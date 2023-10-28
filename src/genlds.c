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
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "pe.h"
#include "cleanup.h"
#include "common.h"

int genlds(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret           = EXIT_SUCCESS;
    FILE   *fh            = NULL;
    FILE   *ofh           = stdout;
    int8_t *image         = NULL;
    char   inputname[256] = { '\0' };

    FAIL_IF(argc < 2, "usage: petool genlds <image> [ofile]\n");

    uint32_t length;
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "rb"));

    if (argc > 2)
    {
        FAIL_IF(file_exists(argv[2]), "%s: output file already exists.\n", argv[2]);
        ofh = fopen(argv[2], "w");
        FAIL_IF_PERROR(ofh == NULL, "%s");
    }

    fclose(fh);
    fh = NULL; // for cleanup

    PIMAGE_DOS_HEADER dos_hdr = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr = (void *)(image + dos_hdr->e_lfanew);

    FAIL_IF(length < 512,                            "File too small.\n");

    if (nt_hdr->Signature != IMAGE_NT_SIGNATURE)
    {
        nt_hdr = (void *)(image - 4);
        FAIL_IF(nt_hdr->FileHeader.Machine != 0x014C, "No valid signatures found.\n");
    }

    strncpy(inputname, file_basename(argv[1]), sizeof(inputname) - 1);
    char* p = strrchr(inputname, '.');
    if (p)
    {
        strncpy(p, ".dat", sizeof(inputname) - strlen(inputname) - 1);
    }

    fprintf(ofh, "/* GNU ld linker script for %s */\n", inputname);

    if (strcmp(argv[0], "genlds") == 0)
    {
        fprintf(ofh, "_start = 0x%"PRIX32";\n", nt_hdr->OptionalHeader.ImageBase + nt_hdr->OptionalHeader.AddressOfEntryPoint);
    }

    fprintf(ofh, "ENTRY(_start);\n");
    fprintf(ofh, "SEARCH_DIR(\"=/w64devkit/i686-w64-mingw32/lib\");\n");
    fprintf(ofh, "SEARCH_DIR(\"=/w64devkit/lib\");\n");
    fprintf(ofh, "SEARCH_DIR(\"=/mingw32/i686-w64-mingw32/lib\");\n");
    fprintf(ofh, "SEARCH_DIR(\"=/mingw32/lib\");\n");
    fprintf(ofh, "SEARCH_DIR(\"=/usr/local/lib\");\n");
    fprintf(ofh, "SEARCH_DIR(\"=/lib\");\n");
    fprintf(ofh, "SEARCH_DIR(\"=/usr/lib\");\n");
    fprintf(ofh, "SECTIONS\n");
    fprintf(ofh, "{\n");

    uint16_t filln = 0;
    bool got_crt_section = false;

    char align[64];
    sprintf(align, "ALIGN(0x%-4"PRIX32")", nt_hdr->OptionalHeader.SectionAlignment);

    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        const PIMAGE_SECTION_HEADER cur_sct = IMAGE_FIRST_SECTION(nt_hdr) + i;
        char buf[9];
        memset(buf, 0, sizeof buf);
        memcpy(buf, cur_sct->Name, 8);

        if (cur_sct->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA && !(cur_sct->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)) {
            fprintf(ofh, "    /DISCARD/                  : { %s(%s) }\n", inputname, buf);
            fprintf(ofh, "    %-15s   0x%-6"PRIX32" : { . = . + 0x%"PRIX32"; }\n", buf, cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase, cur_sct->Misc.VirtualSize ? cur_sct->Misc.VirtualSize : cur_sct->SizeOfRawData);
            continue;
        }

        /* resource section is not directly recompilable even if it doesn't move, use re2obj command instead */
        /* relocation section is not currently supported so we'll remove it */
        if (strcmp(buf, ".rsrc") == 0 || strcmp(buf, ".reloc") == 0) {
            fprintf(ofh, "    /DISCARD/                  : { %s(%s) }\n", inputname, buf);


            if (i < nt_hdr->FileHeader.NumberOfSections - 1) {
                sprintf(buf, "FILL%d", filln++);
                fprintf(ofh, "    %-15s   0x%-6"PRIX32" : { . = . + 0x%"PRIX32"; }\n", buf, cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase, cur_sct->Misc.VirtualSize ? cur_sct->Misc.VirtualSize : cur_sct->SizeOfRawData);
            }

            continue;
        }

        if (strcmp(buf, ".CRT") == 0) {
            /* We need to squeeze it in here, otherwise it doesn't work (EV Nova) */

            fprintf(ofh, "\n");
            fprintf(ofh, "  %s 0x%-6"PRIX32" :\n", buf, cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase);
            fprintf(ofh, "  {\n");
            fprintf(ofh, "    %s(%s)\n", inputname, buf);
            fprintf(ofh, "    ___crt_xc_start__ = . ;\n");
            fprintf(ofh, "    KEEP (*(SORT(.CRT$XC*)))  /* C initialization */\n");
            fprintf(ofh, "    ___crt_xc_end__ = . ;\n");
            fprintf(ofh, "    ___crt_xi_start__ = . ;\n");
            fprintf(ofh, "    KEEP (*(SORT(.CRT$XI*)))  /* C++ initialization */\n");
            fprintf(ofh, "    ___crt_xi_end__ = . ;\n");
            fprintf(ofh, "    ___crt_xl_start__ = . ;\n");
            fprintf(ofh, "    KEEP (*(SORT(.CRT$XL*)))  /* TLS callbacks */\n");
            fprintf(ofh, "    /* ___crt_xl_end__ is defined in the TLS Directory support code */\n");
            fprintf(ofh, "    ___crt_xp_start__ = . ;\n");
            fprintf(ofh, "    KEEP (*(SORT(.CRT$XP*)))  /* Pre-termination */\n");
            fprintf(ofh, "    ___crt_xp_end__ = . ;\n");
            fprintf(ofh, "    ___crt_xt_start__ = . ;\n");
            fprintf(ofh, "    KEEP (*(SORT(.CRT$XT*)))  /* Termination */\n");
            fprintf(ofh, "    ___crt_xt_end__ = . ;\n");
            fprintf(ofh, "  }\n\n");

            got_crt_section = true;
            continue;
        }

        if (cur_sct->Misc.VirtualSize > cur_sct->SizeOfRawData) {

            /* Borland Fix - VirtualSize is aligned to SectionAlignment */

            uint32_t aligned_raw_size = cur_sct->SizeOfRawData;

            while (aligned_raw_size % nt_hdr->OptionalHeader.SectionAlignment)
                aligned_raw_size++;

            fprintf(ofh, "    %-15s   0x%-6"PRIX32" : { %s(%s) . = ALIGN(0x%"PRIX32"); }\n", buf, cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase, inputname, buf, nt_hdr->OptionalHeader.SectionAlignment);

            if (cur_sct->Misc.VirtualSize > aligned_raw_size) {
                fprintf(ofh, "    .bss      %16s : { . = . + 0x%"PRIX32"; }\n", align, cur_sct->Misc.VirtualSize - aligned_raw_size);
            }

            continue;
        }

        fprintf(ofh, "    %-15s   0x%-6"PRIX32" : { %s(%s) }\n", buf, cur_sct->VirtualAddress + nt_hdr->OptionalHeader.ImageBase, inputname, buf);
    }

    fprintf(ofh, "\n");

    fprintf(ofh, "    /DISCARD/                  : { *(.rdata$zzz) }\n\n");

    fprintf(ofh, "  .p_text BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    KEEP (*(SORT_NONE(.init)))\n");
    fprintf(ofh, "    *(.text)\n");
    fprintf(ofh, "    *(SORT(.text$*))\n");
    fprintf(ofh, "     *(.text.*)\n");
    fprintf(ofh, "     *(.gnu.linkonce.t.*)\n");
    fprintf(ofh, "    *(.glue_7t)\n");
    fprintf(ofh, "    *(.glue_7)\n");
    fprintf(ofh, "       /* Note: we always define __CTOR_LIST__ and ___CTOR_LIST__ here,\n");
    fprintf(ofh, "          we do not PROVIDE them.  This is because the ctors.o startup\n");
    fprintf(ofh, "          code in libgcc defines them as common symbols, with the\n");
    fprintf(ofh, "          expectation that they will be overridden by the definitions\n");
    fprintf(ofh, "          here.  If we PROVIDE the symbols then they will not be\n");
    fprintf(ofh, "          overridden and global constructors will not be run.\n");
    fprintf(ofh, "          See PR 22762 for more details.\n");
    fprintf(ofh, "\n");
    fprintf(ofh, "          This does mean that it is not possible for a user to define\n");
    fprintf(ofh, "          their own __CTOR_LIST__ and __DTOR_LIST__ symbols; if they do,\n");
    fprintf(ofh, "          the content from those variables are included but the symbols\n");
    fprintf(ofh, "          defined here silently take precedence.  If they truly need to\n");
    fprintf(ofh, "          be redefined, a custom linker script will have to be used.\n");
    fprintf(ofh, "          (The custom script can just be a copy of this script with the\n");
    fprintf(ofh, "          PROVIDE() qualifiers added).\n");
    fprintf(ofh, "          In particular this means that ld -Ur does not work, because\n");
    fprintf(ofh, "          the proper __CTOR_LIST__ set by ld -Ur is overridden by a\n");
    fprintf(ofh, "          bogus __CTOR_LIST__ set by the final link.  See PR 46.  */\n");
    fprintf(ofh, "       ___CTOR_LIST__ = .;\n");
    fprintf(ofh, "       __CTOR_LIST__ = .;\n");
    fprintf(ofh, "       LONG (-1);\n");
    fprintf(ofh, "       KEEP(*(.ctors));\n");
    fprintf(ofh, "       KEEP(*(.ctor));\n");
    fprintf(ofh, "       KEEP(*(SORT_BY_NAME(.ctors.*)));\n");
    fprintf(ofh, "       LONG (0);\n");
    fprintf(ofh, "       /* See comment about __CTOR_LIST__ above.  The same reasoning\n");
    fprintf(ofh, "          applies here too.  */\n");
    fprintf(ofh, "       ___DTOR_LIST__ = .;\n");
    fprintf(ofh, "       __DTOR_LIST__ = .;\n");
    fprintf(ofh, "       LONG (-1);\n");
    fprintf(ofh, "       KEEP(*(.dtors));\n");
    fprintf(ofh, "       KEEP(*(.dtor));\n");
    fprintf(ofh, "       KEEP(*(SORT_BY_NAME(.dtors.*)));\n");
    fprintf(ofh, "       LONG (0);\n");
    fprintf(ofh, "    KEEP (*(SORT_NONE(.fini)))\n");
    fprintf(ofh, "    /* ??? Why is .gcc_exc here?  */\n");
    fprintf(ofh, "     *(.gcc_exc)\n");
    fprintf(ofh, "    PROVIDE (etext = .);\n");
    fprintf(ofh, "    PROVIDE (_etext = .);\n");
    fprintf(ofh, "     KEEP (*(.gcc_except_table))\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  \n");
    fprintf(ofh, "  /* The Cygwin32 library uses a section to avoid copying certain data\n");
    fprintf(ofh, "     on fork.  This used to be named \".data\".  The linker used\n");
    fprintf(ofh, "     to include this between __data_start__ and __data_end__, but that\n");
    fprintf(ofh, "     breaks building the cygwin32 dll.  Instead, we name the section\n");
    fprintf(ofh, "     \".data_cygwin_nocopy\" and explicitly include it after __data_end__. */\n");
    fprintf(ofh, "  .p_data BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    __data_start__ = . ;\n");
    fprintf(ofh, "    *(.data)\n");
    fprintf(ofh, "    *(.data2)\n");
    fprintf(ofh, "    *(SORT(.data$*))\n");
    fprintf(ofh, "    KEEP(*(.jcr))\n");
    fprintf(ofh, "    __data_end__ = . ;\n");
    fprintf(ofh, "    *(.data_cygwin_nocopy)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .p_rdata BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.rdata)\n");
    fprintf(ofh, "             *(SORT(.rdata$*))\n");
    fprintf(ofh, "    . = ALIGN(4);\n");
    fprintf(ofh, "    __rt_psrelocs_start = .;\n");
    fprintf(ofh, "    KEEP(*(.rdata_runtime_pseudo_reloc))\n");
    fprintf(ofh, "    __rt_psrelocs_end = .;\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  __rt_psrelocs_size = __rt_psrelocs_end - __rt_psrelocs_start;\n");
    fprintf(ofh, "  ___RUNTIME_PSEUDO_RELOC_LIST_END__ = .;\n");
    fprintf(ofh, "  __RUNTIME_PSEUDO_RELOC_LIST_END__ = .;\n");
    fprintf(ofh, "  ___RUNTIME_PSEUDO_RELOC_LIST__ = . - __rt_psrelocs_size;\n");
    fprintf(ofh, "  __RUNTIME_PSEUDO_RELOC_LIST__ = . - __rt_psrelocs_size;\n");
    fprintf(ofh, "  .eh_frame BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    KEEP(*(.eh_frame*))\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .pdata BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    KEEP(*(.pdata*))\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .p_bss BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    __bss_start__ = . ;\n");
    fprintf(ofh, "    *(.bss)\n");
    fprintf(ofh, "    *(COMMON)\n");
    fprintf(ofh, "    __bss_end__ = . ;\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .edata BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.edata)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  /DISCARD/ :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug$S)\n");
    fprintf(ofh, "    *(.debug$T)\n");
    fprintf(ofh, "    *(.debug$F)\n");
    fprintf(ofh, "     *(.drectve)\n");
    fprintf(ofh, "     *(.note.GNU-stack)\n");
    fprintf(ofh, "     *(.gnu.lto_*)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .p_idata BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    __p_idata_start__ = . ;\n");
    fprintf(ofh, "    /* This cannot currently be handled with grouped sections.\n");
    fprintf(ofh, "        See pe.em:sort_sections.  */\n");
    fprintf(ofh, "    KEEP (SORT(*)(.idata$2))\n");
    fprintf(ofh, "    KEEP (SORT(*)(.idata$3))\n");
    fprintf(ofh, "    /* These zeroes mark the end of the import list.  */\n");
    fprintf(ofh, "    LONG (0); LONG (0); LONG (0); LONG (0); LONG (0);\n");
    fprintf(ofh, "    KEEP (SORT(*)(.idata$4))\n");
    fprintf(ofh, "    __IAT_start__ = .;\n");
    fprintf(ofh, "    KEEP (SORT(*)(.idata$5))\n");
    fprintf(ofh, "    __IAT_end__ = .;\n");
    fprintf(ofh, "    KEEP (SORT(*)(.idata$6))\n");
    fprintf(ofh, "    KEEP (SORT(*)(.idata$7))\n");
    fprintf(ofh, "  }\n");

    if (!got_crt_section)
    {
        fprintf(ofh, "  .CRT BLOCK(__section_alignment__) :\n");
        fprintf(ofh, "  {\n");
        fprintf(ofh, "    ___crt_xc_start__ = . ;\n");
        fprintf(ofh, "    KEEP (*(SORT(.CRT$XC*)))  /* C initialization */\n");
        fprintf(ofh, "    ___crt_xc_end__ = . ;\n");
        fprintf(ofh, "    ___crt_xi_start__ = . ;\n");
        fprintf(ofh, "    KEEP (*(SORT(.CRT$XI*)))  /* C++ initialization */\n");
        fprintf(ofh, "    ___crt_xi_end__ = . ;\n");
        fprintf(ofh, "    ___crt_xl_start__ = . ;\n");
        fprintf(ofh, "    KEEP (*(SORT(.CRT$XL*)))  /* TLS callbacks */\n");
        fprintf(ofh, "    /* ___crt_xl_end__ is defined in the TLS Directory support code */\n");
        fprintf(ofh, "    ___crt_xp_start__ = . ;\n");
        fprintf(ofh, "    KEEP (*(SORT(.CRT$XP*)))  /* Pre-termination */\n");
        fprintf(ofh, "    ___crt_xp_end__ = . ;\n");
        fprintf(ofh, "    ___crt_xt_start__ = . ;\n");
        fprintf(ofh, "    KEEP (*(SORT(.CRT$XT*)))  /* Termination */\n");
        fprintf(ofh, "    ___crt_xt_end__ = . ;\n");
        fprintf(ofh, "  }\n");
    }
    
    fprintf(ofh, "  /* Windows TLS expects .tls$AAA to be at the start and .tls$ZZZ to be\n");
    fprintf(ofh, "     at the end of section.  This is important because _tls_start MUST\n");
    fprintf(ofh, "     be at the beginning of the section to enable SECREL32 relocations with TLS\n");
    fprintf(ofh, "     data.  */\n");
    fprintf(ofh, "  .p_tls BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    ___tls_start__ = . ;\n");
    fprintf(ofh, "    KEEP (*(.tls$AAA))\n");
    fprintf(ofh, "    KEEP (*(.tls))\n");
    fprintf(ofh, "    KEEP (*(.tls$))\n");
    fprintf(ofh, "    KEEP (*(SORT(.tls$*)))\n");
    fprintf(ofh, "    KEEP (*(.tls$ZZZ))\n");
    fprintf(ofh, "    ___tls_end__ = . ;\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .endjunk BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    /* end is deprecated, don't use it */\n");
    fprintf(ofh, "    PROVIDE (end = .);\n");
    fprintf(ofh, "    PROVIDE ( _end = .);\n");
    fprintf(ofh, "     __end__ = .;\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .rsrc BLOCK(__section_alignment__) : SUBALIGN(4)\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    KEEP (*(.rsrc))\n");
    fprintf(ofh, "    KEEP (*(.rsrc$*))\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .p_reloc BLOCK(__section_alignment__) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.reloc)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .stab BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.stab)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .stabstr BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.stabstr)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "    \n");
    fprintf(ofh, "  /* DWARF debug sections.\n");
    fprintf(ofh, "     Symbols in the DWARF debugging sections are relative to the beginning\n");
    fprintf(ofh, "     of the section.  Unlike other targets that fake this by putting the\n");
    fprintf(ofh, "     section VMA at 0, the PE format will not allow it.  */\n");
    fprintf(ofh, "  /* DWARF 1.1 and DWARF 2.  */\n");
    fprintf(ofh, "  .debug_aranges BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_aranges)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_aranges BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_aranges)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_pubnames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_pubnames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_pubnames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_pubnames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  /* DWARF 2.  */\n");
    fprintf(ofh, "  .debug_info BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_info .gnu.linkonce.wi.*)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_info BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_info .zdebug.gnu.linkonce.wi.*)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_abbrev BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_abbrev)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_abbrev BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_abbrev)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_line BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_line)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_line BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_line)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_frame BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_frame*)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_frame BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_frame*)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_str BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_str)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_str BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_str)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_loc BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_loc)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_loc BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_loc)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_macinfo BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_macinfo)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_macinfo BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_macinfo)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  /* SGI/MIPS DWARF 2 extensions.  */\n");
    fprintf(ofh, "  .debug_weaknames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_weaknames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_weaknames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_weaknames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_funcnames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_funcnames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_funcnames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_funcnames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_typenames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_typenames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_typenames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_typenames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_varnames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_varnames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_varnames BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_varnames)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  /* DWARF 3.  */\n");
    fprintf(ofh, "  .debug_pubtypes BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_pubtypes)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_pubtypes BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_pubtypes)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_ranges BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_ranges)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_ranges BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_ranges)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  /* DWARF 4.  */\n");
    fprintf(ofh, "  .debug_types BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_types .gnu.linkonce.wt.*)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_types BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_types .gnu.linkonce.wt.*)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  /* DWARF 5.  */\n");
    fprintf(ofh, "  .debug_addr BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_addr)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_addr BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_addr)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_line_str BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_line_str)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_line_str BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_line_str)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_loclists BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_loclists)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_loclists BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_loclists)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_macro BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_macro)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_macro BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_macro)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_names BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_names)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_names BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_names)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_rnglists BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_rnglists)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_rnglists BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_rnglists)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_str_offsets BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_str_offsets)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_str_offsets BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_str_offsets)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .debug_sup BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_sup)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  /* For Go and Rust.  */\n");
    fprintf(ofh, "  .debug_gdb_scripts BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.debug_gdb_scripts)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .zdebug_gdb_scripts BLOCK(__section_alignment__) (NOLOAD) :\n");
    fprintf(ofh, "  {\n");
    fprintf(ofh, "    *(.zdebug_gdb_scripts)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "  .patch BLOCK(__section_alignment__) : \n");
    fprintf(ofh, "  { \n");
    fprintf(ofh, "    *(.patch)\n");
    fprintf(ofh, "  }\n");
    fprintf(ofh, "}\n");
    fprintf(ofh, "\n");

cleanup:
    if (image) free(image);
    if (fh)    fclose(fh);
    if (argc > 2)
    {
        if (ofh)   fclose(ofh);
    }
    return ret;
}
