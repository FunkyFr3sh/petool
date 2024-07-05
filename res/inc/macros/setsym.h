
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
        ".if " #arg_count " >= 5;"                   \
            "_" #name ":;"                           \
            "push " #addr ";"                        \
            "push " #arg_count ";"                   \
            "jmp _watcall;"                          \
        ".else;"                                     \
            "_" #name ":;"                           \
            "push ebx;"                              \
            ".if " #arg_count " >= 4;"               \
                "mov ecx, [esp+20];"                 \
            ".endif;"                                \
            ".if " #arg_count " >= 3;"               \
                "mov ebx, [esp+16];"                 \
            ".endif;"                                \
            ".if " #arg_count " >= 2;"               \
                "mov edx, [esp+12];"                 \
            ".endif;"                                \
            ".if " #arg_count " >= 1;"               \
                "mov eax, [esp+8];"                  \
            ".endif;"                                \
            "call " #addr ";"                        \
            "pop ebx;"                               \
            "ret;"                                   \
        ".endif;"                                    \
    )
