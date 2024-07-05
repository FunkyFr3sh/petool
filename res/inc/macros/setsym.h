
#define SETCGLOB(addr, name)                         \
    __asm (                                          \
        ".global _" #name ";"                        \
        ".equ _" #name ", " #addr ";"                \
    )

// watcom - make existing watcall functions global - requires watcall.asm
#define SETWATGLOB(addr, name, arg_count)            \
    __asm (                                          \
        ".global _" #name ";"                        \
        ".global " #name ";"                         \
        ".equ " #name ", " #addr ";"                 \
        ".section .text;"                            \
        ".align 8, 0xCC;"                            \
        ".ifc " #arg_count ", 0;"                    \
            "_" #name ":;"                           \
            "push ebx;"                              \
            "call " #addr ";"                        \
            "pop ebx;"                               \
            "ret;"                                   \
        ".else;"                                     \
        ".ifc " #arg_count ", 1;"                    \
            "_" #name ":;"                           \
            "push ebx;"                              \
            "mov eax, dword[esp+4];"                 \
            "call " #addr ";"                        \
            "pop ebx;"                               \
            "ret;"                                   \
        ".else;"                                     \
        ".ifc " #arg_count ", 2;"                    \
            "_" #name ":;"                           \
            "push ebx;"                              \
            "mov edx, dword[esp+8];"                 \
            "mov eax, dword[esp+4];"                 \
            "call " #addr ";"                        \
            "pop ebx;"                               \
            "ret;"                                   \
        ".else;"                                     \
        ".ifc " #arg_count ", 3;"                    \
            "_" #name ":;"                           \
            "push ebx;"                              \
            "mov ebx, dword[esp+12];"                \
            "mov edx, dword[esp+8];"                 \
            "mov eax, dword[esp+4];"                 \
            "call " #addr ";"                        \
            "pop ebx;"                               \
            "ret;"                                   \
        ".else;"                                     \
        ".ifc " #arg_count ", 4;"                    \
            "_" #name ":;"                           \
            "push ebx;"                              \
            "mov ecx, dword[esp+16];"                \
            "mov ebx, dword[esp+12];"                \
            "mov edx, dword[esp+8];"                 \
            "mov eax, dword[esp+4];"                 \
            "call " #addr ";"                        \
            "pop ebx;"                               \
            "ret;"                                   \
        ".else;"                                     \
            "_" #name ":;"                           \
            "push " #addr ";"                        \
            "push " #arg_count ";"                   \
            "jmp _watcall;"                          \
        ".endif;"                                    \
        ".endif;"                                    \
        ".endif;"                                    \
        ".endif;"                                    \
        ".endif;"                                    \
    )
