#include <windows.h>
#include <stdio.h>
#include "imports.h"

extern char _p_idata_start__, _image_base__;

int __attribute__((optimize("O0"))) imports_strcmp(const char* s1, const char* s2)
{
    register unsigned char u1, u2;

    while (TRUE)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }
    return 0;
}

FARPROC __attribute__((optimize("O0"))) imports_get_proc_address(HMODULE hModule, LPCSTR lpProcName)
{
    if (hModule == NULL)
        return NULL;

    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hModule;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return NULL;

    PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((DWORD)dos_header + (DWORD)dos_header->e_lfanew);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
        return NULL;

    DWORD export_dir_rva = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    DWORD export_dir_size = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

    if (export_dir_rva == 0 || export_dir_size == 0)
        return NULL;

    PIMAGE_EXPORT_DIRECTORY export_dir = (PIMAGE_EXPORT_DIRECTORY)((DWORD)dos_header + export_dir_rva);

    if (export_dir->AddressOfFunctions == 0 || export_dir->AddressOfNames == 0 || export_dir->AddressOfNameOrdinals == 0)
        return NULL;

    DWORD* functions = (DWORD*)((DWORD)dos_header + export_dir->AddressOfFunctions);
    DWORD* names = (DWORD*)((DWORD)dos_header + export_dir->AddressOfNames);
    WORD* ordinals = (WORD*)((DWORD)dos_header + export_dir->AddressOfNameOrdinals);

    for (int i = 0; i < export_dir->NumberOfNames; i++)
    {
        char* name = (char*)((DWORD)dos_header + names[i]);

        if (imports_strcmp(lpProcName, name) == 0)
        {
            char* func = (void*)((DWORD)dos_header + functions[ordinals[i]]);

            // is forwarder?
            if (func > (char*)export_dir && func < (char*)export_dir + export_dir_size)
                return NULL;

            return (FARPROC)func;
        }
    }

    return NULL;
}

BOOL __attribute__((optimize("O0"))) imports_init()
{
    FARPROC(WINAPI * get_proc_address)(HMODULE, LPCSTR) =
        (void*)imports_get_proc_address(LoadLibraryA_p("kernel32.dll"), "GetProcAddress");

    if (!get_proc_address)
        return FALSE;

    HMODULE(WINAPI * load_library)(LPCSTR) =
        (void*)get_proc_address(LoadLibraryA_p("kernel32.dll"), "LoadLibraryA");

    if (!load_library)
        return FALSE;

    char* failed_mod = NULL;
    char* failed_func = NULL;

    PIMAGE_IMPORT_DESCRIPTOR import_desc = (void*)&_p_idata_start__;

    while (import_desc->FirstThunk)
    {
        if (import_desc->Name)
        {
            char* mod_name = (char*)((DWORD)&_image_base__ + import_desc->Name);

            HMODULE mod = load_library(mod_name);

            if (mod)
            {
                PIMAGE_THUNK_DATA first_thunk = (void*)((DWORD)&_image_base__ + import_desc->FirstThunk);

                while (first_thunk->u1.AddressOfData)
                {
                    if ((first_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32) == 0)
                    {
                        PIMAGE_IMPORT_BY_NAME func = (void*)((DWORD)&_image_base__ + first_thunk->u1.AddressOfData);

                        first_thunk->u1.Function = (DWORD)get_proc_address(mod, (const char*)func->Name);

                        if (!first_thunk->u1.Function)
                        {
                            failed_mod = mod_name;
                            failed_func = (char*)func->Name;
                        }
                    }
                    else
                    {
                        int ordinal = (first_thunk->u1.Ordinal & ~IMAGE_ORDINAL_FLAG32) & 0xffff;

                        first_thunk->u1.Function = (DWORD)get_proc_address(mod, MAKEINTRESOURCEA(ordinal));

                        if (!first_thunk->u1.Function)
                        {
                            failed_mod = mod_name;
                            failed_func = "Unknown Ordinal";
                        }
                    }

                    first_thunk++;
                }
            }
            else if (!failed_mod)
            {
                failed_mod = mod_name;
            }
        }

        import_desc++;
    }

    if (failed_func)
    {
        char msg[256];
        _snprintf(
            msg,
            sizeof msg,
            "The procedure entry point %s could not be located in the dynamic link library %s.",
            failed_func,
            failed_mod);

        MessageBoxA(NULL, msg, NULL, MB_OK);

        return FALSE;
    }

    if (failed_mod)
    {
        char msg[256];
        _snprintf(
            msg,
            sizeof msg,
            "The program can't start because %s is missing from your computer. "
                "Try reinstalling the program to fix this problem.",
            failed_mod);

        MessageBoxA(NULL, msg, NULL, MB_OK);

        return FALSE;
    }

    return TRUE;
}
