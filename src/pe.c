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

bool is_supported_pe_image(int8_t* image, uint32_t length)
{
    int ret = EXIT_SUCCESS;

    PIMAGE_DOS_HEADER dos_hdr = (void*)image;
    PIMAGE_NT_HEADERS nt_hdr = (void*)(image + dos_hdr->e_lfanew);

    FAIL_IF(length < sizeof(IMAGE_DOS_HEADER), "File too small.\n");
    FAIL_IF(dos_hdr->e_magic != IMAGE_DOS_SIGNATURE, "File DOS signature invalid.\n");
    FAIL_IF(dos_hdr->e_lfanew > length - 4 || dos_hdr->e_lfanew < sizeof(IMAGE_DOS_HEADER), "NT headers not found.\n");
    FAIL_IF(nt_hdr->Signature != IMAGE_NT_SIGNATURE, "File NT signature invalid.\n");
    FAIL_IF(nt_hdr->FileHeader.Machine != IMAGE_FILE_MACHINE_I386, "Machine type is not i386.\n");

    bool is_clr = nt_hdr->OptionalHeader.NumberOfRvaAndSizes > 14 && nt_hdr->OptionalHeader.DataDirectory[14].VirtualAddress;
    FAIL_IF(is_clr, ".NET assembly not supported.\n");

cleanup:
    return ret == EXIT_SUCCESS;
}

uint32_t rva_to_offset(uint32_t address, PIMAGE_NT_HEADERS nt_hdr)
{
    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER sct_hdr = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (sct_hdr->VirtualAddress <= address && address < sct_hdr->VirtualAddress + sct_hdr->SizeOfRawData)
        {
            return sct_hdr->PointerToRawData + (address - sct_hdr->VirtualAddress);
        }
    }

    return address;
}

uint32_t offset_to_rva(uint32_t address, PIMAGE_NT_HEADERS nt_hdr)
{
    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER sct_hdr = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (address >= sct_hdr->PointerToRawData && address < sct_hdr->PointerToRawData + sct_hdr->SizeOfRawData)
        {
            return (address - sct_hdr->PointerToRawData) + sct_hdr->VirtualAddress;
        }
    }

    return (1 << 31);
}

bool section_from_offset(uint32_t address, PIMAGE_NT_HEADERS nt_hdr, char* section, uint32_t size)
{
    if (size < 9)
        return false;

    memset(section, 0, size);

    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER sct_hdr = IMAGE_FIRST_SECTION(nt_hdr) + i;

        if (address >= sct_hdr->PointerToRawData && address < sct_hdr->PointerToRawData + sct_hdr->SizeOfRawData)
        {
            memcpy(section, (void*)sct_hdr->Name, 8);
            return true;
        }
    }

    return false;
}
