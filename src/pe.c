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
