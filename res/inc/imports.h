#ifndef IMPORTS_H 
#define IMPORTS_H

#ifdef __cplusplus
extern "C" {
#endif

// ### Types ###


// ### Variables ###


// ### Functions ###

BOOL imports_init();

WINBASEAPI
HMODULE
WINAPI
LoadLibraryA_p(
    _In_ LPCSTR lpLibFileName
);

WINBASEAPI
HMODULE
WINAPI
LoadLibraryW_p(
    _In_ LPCWSTR lpLibFileName
);

WINBASEAPI
HMODULE
WINAPI
LoadLibraryExA(
    _In_ LPCSTR lpLibFileName,
    _Reserved_ HANDLE hFile,
    _In_ DWORD dwFlags
);

WINBASEAPI
HMODULE
WINAPI
LoadLibraryExW(
    _In_ LPCWSTR lpLibFileName,
    _Reserved_ HANDLE hFile,
    _In_ DWORD dwFlags
);

WINBASEAPI
HMODULE
WINAPI
GetModuleHandleA_p(
    __in_opt LPCSTR lpModuleName
);
    
WINBASEAPI
HMODULE
WINAPI
GetModuleHandleW_p(
    __in_opt LPCWSTR lpModuleName
);

WINBASEAPI
FARPROC
WINAPI
GetProcAddress_p(
    _In_ HMODULE hModule,
    _In_ LPCSTR lpProcName
);

#ifdef __cplusplus
};
#endif

#endif
