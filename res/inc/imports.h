#ifndef IMPORTS_H 
#define IMPORTS_H

// This header is used for patches
// This header will be split up as it becomes larger

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

#endif
