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

        if (sct_hdr->VirtualAddress + nt_hdr->OptionalHeader.ImageBase <= address && address < sct_hdr->VirtualAddress + nt_hdr->OptionalHeader.ImageBase + sct_hdr->SizeOfRawData)
        {
            return sct_hdr->PointerToRawData + (address - (sct_hdr->VirtualAddress + nt_hdr->OptionalHeader.ImageBase));
        }
    }

    return 0;
}

int import(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    FILE   *fh    = NULL;
    int8_t *image = NULL;
    FILE   *ofh   = stdout;

    FAIL_IF(argc < 2, "usage: petool import <image> [ofile]\n");

    uint32_t length;
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "rb"));

    if (argc > 2)
    {
        FAIL_IF(file_exists(argv[2]), "%s: output file already exists.\n", argv[2]);
        ofh = fopen(argv[2], "w");
        FAIL_IF_PERROR(ofh == NULL, "%s");
    }

    PIMAGE_DOS_HEADER dos_hdr = (void *)image;
    PIMAGE_NT_HEADERS nt_hdr = (void *)(image + dos_hdr->e_lfanew);

    FAIL_IF (nt_hdr->OptionalHeader.NumberOfRvaAndSizes < 2, "Not enough DataDirectories.\n");

    uint32_t offset = rva_to_offset(nt_hdr->OptionalHeader.ImageBase + nt_hdr->OptionalHeader.DataDirectory[1].VirtualAddress, nt_hdr);
    IMAGE_IMPORT_DESCRIPTOR *i = (void *)(image + offset);

    if (strcmp(argv[0], "import") != 0) {
        fprintf(ofh, "%%include \"macros/imports.inc\"\n");
        fprintf(ofh, "\n");
        fprintf(ofh, "\n");
        fprintf(ofh, "ImageBase equ 0x%"PRIX32"\n", nt_hdr->OptionalHeader.ImageBase);
        fprintf(ofh, "\n");
        fprintf(ofh, "; Original Imports - Don't modify (Unless you want to remove a dependency on one of these dlls)\n");

        while (1) {
            if (i->Name == 0) {
                fprintf(ofh, "\n");
                fprintf(ofh, "; Custom Imports\n");
                fprintf(ofh, ";import_library KERNEL32.dll\n");
                fprintf(ofh, "import_library MSVCRT.dll\n");
                fprintf(ofh, "import_library_end\n");
                fprintf(ofh, "\n");
                fprintf(ofh, "\n");
                fprintf(ofh, "; KERNEL32.dll functions (example)\n");
                fprintf(ofh, ";import_function KERNEL32.dll, GetPrivateProfileStringA\n");
                fprintf(ofh, ";import_function KERNEL32.dll, GetPrivateProfileIntA\n");
                fprintf(ofh, ";import_function_end\n");
                fprintf(ofh, "\n");
                fprintf(ofh, "; MSVCRT.dll functions\n");
                fprintf(ofh, "import_function MSVCRT.dll, _abnormal_termination\n");
                fprintf(ofh, "import_function MSVCRT.dll, _access\n");
                fprintf(ofh, "import_function MSVCRT.dll, _acmdln\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fdiv_m16i\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fdiv_m32\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fdiv_m32i\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fdiv_m64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fdiv_r\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fdivr_m16i\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fdivr_m32\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fdivr_m32i\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fdivr_m64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fpatan\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fprem\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fprem1\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adj_fptan\n");
                fprintf(ofh, "import_function MSVCRT.dll, _adjust_fdiv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _aexit_rtn\n");
                fprintf(ofh, "import_function MSVCRT.dll, _aligned_free\n");
                fprintf(ofh, "import_function MSVCRT.dll, _aligned_malloc\n");
                fprintf(ofh, "import_function MSVCRT.dll, _aligned_offset_malloc\n");
                fprintf(ofh, "import_function MSVCRT.dll, _aligned_offset_realloc\n");
                fprintf(ofh, "import_function MSVCRT.dll, _aligned_realloc\n");
                fprintf(ofh, "import_function MSVCRT.dll, _amsg_exit\n");
                fprintf(ofh, "import_function MSVCRT.dll, _assert\n");
                fprintf(ofh, "import_function MSVCRT.dll, _atodbl\n");
                fprintf(ofh, "import_function MSVCRT.dll, _atoi64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _atoldbl\n");
                fprintf(ofh, "import_function MSVCRT.dll, _beep\n");
                fprintf(ofh, "import_function MSVCRT.dll, _beginthread\n");
                fprintf(ofh, "import_function MSVCRT.dll, _beginthreadex\n");
                fprintf(ofh, "import_function MSVCRT.dll, _c_exit\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cabs\n");
                fprintf(ofh, "import_function MSVCRT.dll, _callnewh\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cexit\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cgets\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cgetws\n");
                fprintf(ofh, "import_function MSVCRT.dll, _chdir\n");
                fprintf(ofh, "import_function MSVCRT.dll, _chdrive\n");
                fprintf(ofh, "import_function MSVCRT.dll, _chgsign\n");
                fprintf(ofh, "import_function MSVCRT.dll, _chkesp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _chmod\n");
                fprintf(ofh, "import_function MSVCRT.dll, _chsize\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIacos\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIasin\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIatan\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIatan2\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIcos\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIcosh\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIexp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIfmod\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIlog\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIlog10\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIpow\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIsin\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIsinh\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CIsqrt\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CItan\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CItanh\n");
                fprintf(ofh, "import_function MSVCRT.dll, _clearfp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _close\n");
                fprintf(ofh, "import_function MSVCRT.dll, _commit\n");
                fprintf(ofh, "import_function MSVCRT.dll, _commode\n");
                fprintf(ofh, "import_function MSVCRT.dll, _control87\n");
                fprintf(ofh, "import_function MSVCRT.dll, _controlfp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _copysign\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cputs\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cputws\n");
                fprintf(ofh, "import_function MSVCRT.dll, _creat\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cscanf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ctime64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ctype\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cwait\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cwprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _cwscanf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _CxxThrowException\n");
                fprintf(ofh, "import_function MSVCRT.dll, _daylight\n");
                fprintf(ofh, "import_function MSVCRT.dll, _dstbias\n");
                fprintf(ofh, "import_function MSVCRT.dll, _dup\n");
                fprintf(ofh, "import_function MSVCRT.dll, _dup2\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ecvt\n");
                fprintf(ofh, "import_function MSVCRT.dll, _EH_prolog\n");
                fprintf(ofh, "import_function MSVCRT.dll, _endthread\n");
                fprintf(ofh, "import_function MSVCRT.dll, _endthreadex\n");
                fprintf(ofh, "import_function MSVCRT.dll, _environ\n");
                fprintf(ofh, "import_function MSVCRT.dll, _eof\n");
                fprintf(ofh, "import_function MSVCRT.dll, _errno\n");
                fprintf(ofh, "import_function MSVCRT.dll, _except_handler2\n");
                fprintf(ofh, "import_function MSVCRT.dll, _except_handler3\n");
                fprintf(ofh, "import_function MSVCRT.dll, _execl\n");
                fprintf(ofh, "import_function MSVCRT.dll, _execle\n");
                fprintf(ofh, "import_function MSVCRT.dll, _execlp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _execlpe\n");
                fprintf(ofh, "import_function MSVCRT.dll, _execv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _execve\n");
                fprintf(ofh, "import_function MSVCRT.dll, _execvp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _execvpe\n");
                fprintf(ofh, "import_function MSVCRT.dll, _exit\n");
                fprintf(ofh, "import_function MSVCRT.dll, _expand\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fcloseall\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fcvt\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fdopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fgetchar\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fgetwchar\n");
                fprintf(ofh, "import_function MSVCRT.dll, _filbuf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fileinfo\n");
                fprintf(ofh, "import_function MSVCRT.dll, _filelength\n");
                fprintf(ofh, "import_function MSVCRT.dll, _filelengthi64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fileno\n");
                fprintf(ofh, "import_function MSVCRT.dll, _findclose\n");
                fprintf(ofh, "import_function MSVCRT.dll, _findfirst\n");
                fprintf(ofh, "import_function MSVCRT.dll, _findfirst64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _findfirsti64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _findnext\n");
                fprintf(ofh, "import_function MSVCRT.dll, _findnext64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _findnexti64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _finite\n");
                fprintf(ofh, "import_function MSVCRT.dll, _flsbuf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _flushall\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fmode\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fpclass\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fpieee_flt\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fpreset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fputchar\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fputwchar\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fsopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fstat\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fstat64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fstati64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ftime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ftime64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ftol\n");
                fprintf(ofh, "import_function MSVCRT.dll, _fullpath\n");
                fprintf(ofh, "import_function MSVCRT.dll, _futime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _futime64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _gcvt\n");
                fprintf(ofh, "import_function MSVCRT.dll, _get_osfhandle\n");
                fprintf(ofh, "import_function MSVCRT.dll, _get_sbh_threshold\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getch\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getche\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getcwd\n");
                fprintf(ofh, "import_function MSVCRT.dll, _Getdays\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getdcwd\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getdiskfree\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getdllprocaddr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getdrive\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getdrives\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getmaxstdio\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getmbcp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _Getmonths\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getpid\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getsystime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _Gettnames\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getw\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getwch\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getwche\n");
                fprintf(ofh, "import_function MSVCRT.dll, _getws\n");
                fprintf(ofh, "import_function MSVCRT.dll, _global_unwind2\n");
                fprintf(ofh, "import_function MSVCRT.dll, _gmtime64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _heapadd\n");
                fprintf(ofh, "import_function MSVCRT.dll, _heapchk\n");
                fprintf(ofh, "import_function MSVCRT.dll, _heapmin\n");
                fprintf(ofh, "import_function MSVCRT.dll, _heapset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _heapused\n");
                fprintf(ofh, "import_function MSVCRT.dll, _heapwalk\n");
                fprintf(ofh, "import_function MSVCRT.dll, _HUGE\n");
                fprintf(ofh, "import_function MSVCRT.dll, _hypot\n");
                fprintf(ofh, "import_function MSVCRT.dll, _i64toa\n");
                fprintf(ofh, "import_function MSVCRT.dll, _i64tow\n");
                fprintf(ofh, "import_function MSVCRT.dll, _initterm\n");
                fprintf(ofh, "import_function MSVCRT.dll, _inp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _inpd\n");
                fprintf(ofh, "import_function MSVCRT.dll, _inpw\n");
                fprintf(ofh, "import_function MSVCRT.dll, _iob\n");
                fprintf(ofh, "import_function MSVCRT.dll, _isatty\n");
                fprintf(ofh, "import_function MSVCRT.dll, _isctype\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbalnum\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbalpha\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbgraph\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbkalnum\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbkana\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbkprint\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbkpunct\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbblead\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbprint\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbpunct\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbbtrail\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcalnum\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcalpha\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcdigit\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcgraph\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbchira\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbckata\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcl0\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcl1\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcl2\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbclegal\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbclower\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcprint\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcpunct\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcspace\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcsymbol\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbcupper\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbslead\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ismbstrail\n");
                fprintf(ofh, "import_function MSVCRT.dll, _isnan\n");
                fprintf(ofh, "import_function MSVCRT.dll, _itoa\n");
                fprintf(ofh, "import_function MSVCRT.dll, _itow\n");
                fprintf(ofh, "import_function MSVCRT.dll, _j0\n");
                fprintf(ofh, "import_function MSVCRT.dll, _j1\n");
                fprintf(ofh, "import_function MSVCRT.dll, _jn\n");
                fprintf(ofh, "import_function MSVCRT.dll, _kbhit\n");
                fprintf(ofh, "import_function MSVCRT.dll, _lfind\n");
                fprintf(ofh, "import_function MSVCRT.dll, _loaddll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _local_unwind2\n");
                fprintf(ofh, "import_function MSVCRT.dll, _localtime64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _lock\n");
                fprintf(ofh, "import_function MSVCRT.dll, _locking\n");
                fprintf(ofh, "import_function MSVCRT.dll, _logb\n");
                fprintf(ofh, "import_function MSVCRT.dll, _longjmpex\n");
                fprintf(ofh, "import_function MSVCRT.dll, _lrotl\n");
                fprintf(ofh, "import_function MSVCRT.dll, _lrotr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _lsearch\n");
                fprintf(ofh, "import_function MSVCRT.dll, _lseek\n");
                fprintf(ofh, "import_function MSVCRT.dll, _lseeki64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _makepath\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbbtombc\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbbtype\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbcasemap\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbccpy\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbcjistojms\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbcjmstojis\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbclen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbctohira\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbctokata\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbctolower\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbctombb\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbctoupper\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbctype\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsbtype\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbscat\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbschr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbscmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbscoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbscpy\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbscspn\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsdec\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsdup\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsicmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsicoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsinc\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbslen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbslwr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnbcat\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnbcmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnbcnt\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnbcoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnbcpy\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnbicmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnbicoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnbset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsncat\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnccnt\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsncmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsncoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsncpy\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnextc\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnicmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnicoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsninc\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsnset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbspbrk\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsrchr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsrev\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsspn\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsspnp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsstr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbstok\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbstrlen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mbsupr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _memccpy\n");
                fprintf(ofh, "import_function MSVCRT.dll, _memicmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mkdir\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mktemp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _mktime64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _msize\n");
                fprintf(ofh, "import_function MSVCRT.dll, _nextafter\n");
                fprintf(ofh, "import_function MSVCRT.dll, _onexit\n");
                fprintf(ofh, "import_function MSVCRT.dll, _open\n");
                fprintf(ofh, "import_function MSVCRT.dll, _open_osfhandle\n");
                fprintf(ofh, "import_function MSVCRT.dll, _osplatform\n");
                fprintf(ofh, "import_function MSVCRT.dll, _osver\n");
                fprintf(ofh, "import_function MSVCRT.dll, _outp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _outpd\n");
                fprintf(ofh, "import_function MSVCRT.dll, _outpw\n");
                fprintf(ofh, "import_function MSVCRT.dll, _pclose\n");
                fprintf(ofh, "import_function MSVCRT.dll, _pctype\n");
                fprintf(ofh, "import_function MSVCRT.dll, _pgmptr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _pipe\n");
                fprintf(ofh, "import_function MSVCRT.dll, _popen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _purecall\n");
                fprintf(ofh, "import_function MSVCRT.dll, _putch\n");
                fprintf(ofh, "import_function MSVCRT.dll, _putenv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _putw\n");
                fprintf(ofh, "import_function MSVCRT.dll, _putwch\n");
                fprintf(ofh, "import_function MSVCRT.dll, _putws\n");
                fprintf(ofh, "import_function MSVCRT.dll, _pwctype\n");
                fprintf(ofh, "import_function MSVCRT.dll, _read\n");
                fprintf(ofh, "import_function MSVCRT.dll, _resetstkoflw\n");
                fprintf(ofh, "import_function MSVCRT.dll, _rmdir\n");
                fprintf(ofh, "import_function MSVCRT.dll, _rmtmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _safe_fdiv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _safe_fdivr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _safe_fprem\n");
                fprintf(ofh, "import_function MSVCRT.dll, _safe_fprem1\n");
                fprintf(ofh, "import_function MSVCRT.dll, _scalb\n");
                fprintf(ofh, "import_function MSVCRT.dll, _scprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _scwprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _searchenv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _seh_longjmp_unwind\n");
                fprintf(ofh, "import_function MSVCRT.dll, _set_error_mode\n");
                fprintf(ofh, "import_function MSVCRT.dll, _set_sbh_threshold\n");
                fprintf(ofh, "import_function MSVCRT.dll, _set_SSE2_enable\n");
                fprintf(ofh, "import_function MSVCRT.dll, _seterrormode\n");
                fprintf(ofh, "import_function MSVCRT.dll, _setjmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _setjmp3\n");
                fprintf(ofh, "import_function MSVCRT.dll, _setmaxstdio\n");
                fprintf(ofh, "import_function MSVCRT.dll, _setmbcp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _setmode\n");
                fprintf(ofh, "import_function MSVCRT.dll, _setsystime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _sleep\n");
                fprintf(ofh, "import_function MSVCRT.dll, _snprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _snscanf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _snwprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _snwscanf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _sopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _spawnl\n");
                fprintf(ofh, "import_function MSVCRT.dll, _spawnle\n");
                fprintf(ofh, "import_function MSVCRT.dll, _spawnlp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _spawnlpe\n");
                fprintf(ofh, "import_function MSVCRT.dll, _spawnv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _spawnve\n");
                fprintf(ofh, "import_function MSVCRT.dll, _spawnvp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _spawnvpe\n");
                fprintf(ofh, "import_function MSVCRT.dll, _splitpath\n");
                fprintf(ofh, "import_function MSVCRT.dll, _stat\n");
                fprintf(ofh, "import_function MSVCRT.dll, _stat64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _stati64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _statusfp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strcmpi\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strdate\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strerror\n");
                fprintf(ofh, "import_function MSVCRT.dll, _Strftime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _stricoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strlwr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strncoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strnicmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strnicoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strnset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strrev\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strtime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strtoi64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strtoui64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _strupr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _swab\n");
                fprintf(ofh, "import_function MSVCRT.dll, _sys_errlist\n");
                fprintf(ofh, "import_function MSVCRT.dll, _sys_nerr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _tell\n");
                fprintf(ofh, "import_function MSVCRT.dll, _telli64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _tempnam\n");
                fprintf(ofh, "import_function MSVCRT.dll, _time64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _timezone\n");
                fprintf(ofh, "import_function MSVCRT.dll, _tolower\n");
                fprintf(ofh, "import_function MSVCRT.dll, _toupper\n");
                fprintf(ofh, "import_function MSVCRT.dll, _tzname\n");
                fprintf(ofh, "import_function MSVCRT.dll, _tzset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ui64toa\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ui64tow\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ultoa\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ultow\n");
                fprintf(ofh, "import_function MSVCRT.dll, _umask\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ungetch\n");
                fprintf(ofh, "import_function MSVCRT.dll, _ungetwch\n");
                fprintf(ofh, "import_function MSVCRT.dll, _unlink\n");
                fprintf(ofh, "import_function MSVCRT.dll, _unloaddll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _unlock\n");
                fprintf(ofh, "import_function MSVCRT.dll, _utime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _utime64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _vscprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _vscwprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _vsnprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _vsnwprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, _waccess\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wasctime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wchdir\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wchmod\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcmdln\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcreat\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsdup\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcserror\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsicmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsicoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcslwr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsncoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsnicmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsnicoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsnset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsrev\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsset\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcstoi64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcstoui64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wcsupr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wctime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wctime64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wenviron\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wexecl\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wexecle\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wexeclp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wexeclpe\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wexecv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wexecve\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wexecvp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wexecvpe\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfdopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfindfirst\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfindfirst64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfindfirsti64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfindnext\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfindnext64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfindnexti64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfreopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfsopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wfullpath\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wgetcwd\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wgetdcwd\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wgetenv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _winmajor\n");
                fprintf(ofh, "import_function MSVCRT.dll, _winminor\n");
                fprintf(ofh, "import_function MSVCRT.dll, _winver\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wmakepath\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wmkdir\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wmktemp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wperror\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wpgmptr\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wpopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wputenv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wremove\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wrename\n");
                fprintf(ofh, "import_function MSVCRT.dll, _write\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wrmdir\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wsearchenv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wsetlocale\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wsopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wspawnl\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wspawnle\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wspawnlp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wspawnlpe\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wspawnv\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wspawnve\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wspawnvp\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wspawnvpe\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wsplitpath\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wstat\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wstat64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wstati64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wstrdate\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wstrtime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wsystem\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wtempnam\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wtmpnam\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wtof\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wtoi\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wtoi64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wtol\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wunlink\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wutime\n");
                fprintf(ofh, "import_function MSVCRT.dll, _wutime64\n");
                fprintf(ofh, "import_function MSVCRT.dll, _XcptFilter\n");
                fprintf(ofh, "import_function MSVCRT.dll, _y0\n");
                fprintf(ofh, "import_function MSVCRT.dll, _y1\n");
                fprintf(ofh, "import_function MSVCRT.dll, _yn\n");
                fprintf(ofh, "import_function MSVCRT.dll, abort\n");
                fprintf(ofh, "import_function MSVCRT.dll, abs\n");
                fprintf(ofh, "import_function MSVCRT.dll, acos\n");
                fprintf(ofh, "import_function MSVCRT.dll, asctime\n");
                fprintf(ofh, "import_function MSVCRT.dll, asin\n");
                fprintf(ofh, "import_function MSVCRT.dll, atan\n");
                fprintf(ofh, "import_function MSVCRT.dll, atan2\n");
                fprintf(ofh, "import_function MSVCRT.dll, atexit\n");
                fprintf(ofh, "import_function MSVCRT.dll, atof\n");
                fprintf(ofh, "import_function MSVCRT.dll, atoi\n");
                fprintf(ofh, "import_function MSVCRT.dll, atol\n");
                fprintf(ofh, "import_function MSVCRT.dll, bsearch\n");
                fprintf(ofh, "import_function MSVCRT.dll, calloc\n");
                fprintf(ofh, "import_function MSVCRT.dll, ceil\n");
                fprintf(ofh, "import_function MSVCRT.dll, clearerr\n");
                fprintf(ofh, "import_function MSVCRT.dll, clock\n");
                fprintf(ofh, "import_function MSVCRT.dll, cos\n");
                fprintf(ofh, "import_function MSVCRT.dll, cosh\n");
                fprintf(ofh, "import_function MSVCRT.dll, ctime\n");
                fprintf(ofh, "import_function MSVCRT.dll, difftime\n");
                fprintf(ofh, "import_function MSVCRT.dll, div\n");
                fprintf(ofh, "import_function MSVCRT.dll, exit\n");
                fprintf(ofh, "import_function MSVCRT.dll, exp\n");
                fprintf(ofh, "import_function MSVCRT.dll, fabs\n");
                fprintf(ofh, "import_function MSVCRT.dll, fclose\n");
                fprintf(ofh, "import_function MSVCRT.dll, feof\n");
                fprintf(ofh, "import_function MSVCRT.dll, ferror\n");
                fprintf(ofh, "import_function MSVCRT.dll, fflush\n");
                fprintf(ofh, "import_function MSVCRT.dll, fgetc\n");
                fprintf(ofh, "import_function MSVCRT.dll, fgetpos\n");
                fprintf(ofh, "import_function MSVCRT.dll, fgets\n");
                fprintf(ofh, "import_function MSVCRT.dll, fgetwc\n");
                fprintf(ofh, "import_function MSVCRT.dll, fgetws\n");
                fprintf(ofh, "import_function MSVCRT.dll, floor\n");
                fprintf(ofh, "import_function MSVCRT.dll, fmod\n");
                fprintf(ofh, "import_function MSVCRT.dll, fopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, fprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, fputc\n");
                fprintf(ofh, "import_function MSVCRT.dll, fputs\n");
                fprintf(ofh, "import_function MSVCRT.dll, fputwc\n");
                fprintf(ofh, "import_function MSVCRT.dll, fputws\n");
                fprintf(ofh, "import_function MSVCRT.dll, fread\n");
                fprintf(ofh, "import_function MSVCRT.dll, free\n");
                fprintf(ofh, "import_function MSVCRT.dll, freopen\n");
                fprintf(ofh, "import_function MSVCRT.dll, frexp\n");
                fprintf(ofh, "import_function MSVCRT.dll, fscanf\n");
                fprintf(ofh, "import_function MSVCRT.dll, fseek\n");
                fprintf(ofh, "import_function MSVCRT.dll, fsetpos\n");
                fprintf(ofh, "import_function MSVCRT.dll, ftell\n");
                fprintf(ofh, "import_function MSVCRT.dll, fwprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, fwrite\n");
                fprintf(ofh, "import_function MSVCRT.dll, fwscanf\n");
                fprintf(ofh, "import_function MSVCRT.dll, getenv\n");
                fprintf(ofh, "import_function MSVCRT.dll, gets\n");
                fprintf(ofh, "import_function MSVCRT.dll, getwc\n");
                fprintf(ofh, "import_function MSVCRT.dll, gmtime\n");
                fprintf(ofh, "import_function MSVCRT.dll, is_wctype\n");
                fprintf(ofh, "import_function MSVCRT.dll, isalnum\n");
                fprintf(ofh, "import_function MSVCRT.dll, isalpha\n");
                fprintf(ofh, "import_function MSVCRT.dll, iscntrl\n");
                fprintf(ofh, "import_function MSVCRT.dll, isdigit\n");
                fprintf(ofh, "import_function MSVCRT.dll, isgraph\n");
                fprintf(ofh, "import_function MSVCRT.dll, isleadbyte\n");
                fprintf(ofh, "import_function MSVCRT.dll, islower\n");
                fprintf(ofh, "import_function MSVCRT.dll, isprint\n");
                fprintf(ofh, "import_function MSVCRT.dll, ispunct\n");
                fprintf(ofh, "import_function MSVCRT.dll, isspace\n");
                fprintf(ofh, "import_function MSVCRT.dll, isupper\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswalnum\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswalpha\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswascii\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswcntrl\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswctype\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswdigit\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswgraph\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswlower\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswprint\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswpunct\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswspace\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswupper\n");
                fprintf(ofh, "import_function MSVCRT.dll, iswxdigit\n");
                fprintf(ofh, "import_function MSVCRT.dll, isxdigit\n");
                fprintf(ofh, "import_function MSVCRT.dll, ldexp\n");
                fprintf(ofh, "import_function MSVCRT.dll, localeconv\n");
                fprintf(ofh, "import_function MSVCRT.dll, localtime\n");
                fprintf(ofh, "import_function MSVCRT.dll, log\n");
                fprintf(ofh, "import_function MSVCRT.dll, log10\n");
                fprintf(ofh, "import_function MSVCRT.dll, longjmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, malloc\n");
                fprintf(ofh, "import_function MSVCRT.dll, mblen\n");
                fprintf(ofh, "import_function MSVCRT.dll, mbstowcs\n");
                fprintf(ofh, "import_function MSVCRT.dll, mbtowc\n");
                fprintf(ofh, "import_function MSVCRT.dll, memchr\n");
                fprintf(ofh, "import_function MSVCRT.dll, memcmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, memcpy\n");
                fprintf(ofh, "import_function MSVCRT.dll, memmove\n");
                fprintf(ofh, "import_function MSVCRT.dll, memset\n");
                fprintf(ofh, "import_function MSVCRT.dll, mktime\n");
                fprintf(ofh, "import_function MSVCRT.dll, modf\n");
                fprintf(ofh, "import_function MSVCRT.dll, perror\n");
                fprintf(ofh, "import_function MSVCRT.dll, pow\n");
                fprintf(ofh, "import_function MSVCRT.dll, printf\n");
                fprintf(ofh, "import_function MSVCRT.dll, putchar\n");
                fprintf(ofh, "import_function MSVCRT.dll, puts\n");
                fprintf(ofh, "import_function MSVCRT.dll, putwc\n");
                fprintf(ofh, "import_function MSVCRT.dll, putwchar\n");
                fprintf(ofh, "import_function MSVCRT.dll, qsort\n");
                fprintf(ofh, "import_function MSVCRT.dll, raise\n");
                fprintf(ofh, "import_function MSVCRT.dll, rand\n");
                fprintf(ofh, "import_function MSVCRT.dll, realloc\n");
                fprintf(ofh, "import_function MSVCRT.dll, remove\n");
                fprintf(ofh, "import_function MSVCRT.dll, rename\n");
                fprintf(ofh, "import_function MSVCRT.dll, rewind\n");
                fprintf(ofh, "import_function MSVCRT.dll, scanf\n");
                fprintf(ofh, "import_function MSVCRT.dll, setbuf\n");
                fprintf(ofh, "import_function MSVCRT.dll, setlocale\n");
                fprintf(ofh, "import_function MSVCRT.dll, setvbuf\n");
                fprintf(ofh, "import_function MSVCRT.dll, signal\n");
                fprintf(ofh, "import_function MSVCRT.dll, sin\n");
                fprintf(ofh, "import_function MSVCRT.dll, sinh\n");
                fprintf(ofh, "import_function MSVCRT.dll, sprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, sqrt\n");
                fprintf(ofh, "import_function MSVCRT.dll, srand\n");
                fprintf(ofh, "import_function MSVCRT.dll, sscanf\n");
                fprintf(ofh, "import_function MSVCRT.dll, strchr\n");
                fprintf(ofh, "import_function MSVCRT.dll, strcmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, strcoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, strcspn\n");
                fprintf(ofh, "import_function MSVCRT.dll, strerror\n");
                fprintf(ofh, "import_function MSVCRT.dll, strftime\n");
                fprintf(ofh, "import_function MSVCRT.dll, strlen\n");
                fprintf(ofh, "import_function MSVCRT.dll, strncat\n");
                fprintf(ofh, "import_function MSVCRT.dll, strncmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, strncpy\n");
                fprintf(ofh, "import_function MSVCRT.dll, strpbrk\n");
                fprintf(ofh, "import_function MSVCRT.dll, strrchr\n");
                fprintf(ofh, "import_function MSVCRT.dll, strspn\n");
                fprintf(ofh, "import_function MSVCRT.dll, strstr\n");
                fprintf(ofh, "import_function MSVCRT.dll, strtod\n");
                fprintf(ofh, "import_function MSVCRT.dll, strtok\n");
                fprintf(ofh, "import_function MSVCRT.dll, strtol\n");
                fprintf(ofh, "import_function MSVCRT.dll, strtoul\n");
                fprintf(ofh, "import_function MSVCRT.dll, strxfrm\n");
                fprintf(ofh, "import_function MSVCRT.dll, swscanf\n");
                fprintf(ofh, "import_function MSVCRT.dll, system\n");
                fprintf(ofh, "import_function MSVCRT.dll, tan\n");
                fprintf(ofh, "import_function MSVCRT.dll, tanh\n");
                fprintf(ofh, "import_function MSVCRT.dll, time\n");
                fprintf(ofh, "import_function MSVCRT.dll, tmpfile\n");
                fprintf(ofh, "import_function MSVCRT.dll, tmpnam\n");
                fprintf(ofh, "import_function MSVCRT.dll, tolower\n");
                fprintf(ofh, "import_function MSVCRT.dll, toupper\n");
                fprintf(ofh, "import_function MSVCRT.dll, towlower\n");
                fprintf(ofh, "import_function MSVCRT.dll, towupper\n");
                fprintf(ofh, "import_function MSVCRT.dll, ungetc\n");
                fprintf(ofh, "import_function MSVCRT.dll, ungetwc\n");
                fprintf(ofh, "import_function MSVCRT.dll, vfprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, vfwprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, vprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, vsprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, vwprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcscat\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcschr\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcscmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcscoll\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcscpy\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcscspn\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcsftime\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcslen\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcsncat\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcsncmp\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcsncpy\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcspbrk\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcsrchr\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcsspn\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcsstr\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcstod\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcstok\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcstol\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcstombs\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcstoul\n");
                fprintf(ofh, "import_function MSVCRT.dll, wcsxfrm\n");
                fprintf(ofh, "import_function MSVCRT.dll, wctomb\n");
                fprintf(ofh, "import_function MSVCRT.dll, wprintf\n");
                fprintf(ofh, "import_function MSVCRT.dll, wscanf\n");
                fprintf(ofh, "import_function_end\n");
                break;
            }

            fprintf(
                ofh, 
                "import_library %s, 0x%"PRIX32", 0x%"PRIX32", 0x%"PRIX32", 0x%"PRIX32"\n",
                (char*)(image + rva_to_offset(nt_hdr->OptionalHeader.ImageBase + i->Name, nt_hdr)),
                i->OriginalFirstThunk,
                i->TimeDateStamp,
                i->ForwarderChain,
                i->FirstThunk);

            i++;
        }
    }
    else {
        fprintf(ofh, "; Imports for %s\n", argv[1]);
        fprintf(ofh, "ImageBase equ 0x%"PRIX32"\n", nt_hdr->OptionalHeader.ImageBase);
        fprintf(ofh, "\n");
        fprintf(ofh, "section .idata\n\n");

        while (1) {
            if (i->Name != 0) {
                char *name = (char *)(image + rva_to_offset(nt_hdr->OptionalHeader.ImageBase + i->Name, nt_hdr));
                fprintf(ofh, "; %s\n", name);
            } else {
                fprintf(ofh, "; END\n");
            }

            fprintf(ofh, "dd 0x%-8"PRIX32" ; OriginalFirstThunk\n", i->OriginalFirstThunk);
            fprintf(ofh, "dd 0x%-8"PRIX32" ; TimeDateStamp\n", i->TimeDateStamp);
            fprintf(ofh, "dd 0x%-8"PRIX32" ; ForwarderChain\n", i->ForwarderChain);
            fprintf(ofh, "dd 0x%-8"PRIX32" ; Name\n", i->Name);
            fprintf(ofh, "dd 0x%-8"PRIX32" ; FirstThunk\n", i->FirstThunk);

            fprintf(ofh, "\n");

            if (i->OriginalFirstThunk == 0)
                break;
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
