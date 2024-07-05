
#define SETCGLOB(addr, name)                         \
    __asm (                                          \
        ".global _" #name ";"                        \
        ".equ _" #name ", " #addr ";"                \
    )

// watcom - make existing watcall functions global
#define SETWATGLOB(addr, name, arg_count)            \
    __asm (                                          \
        ".global _" #name ";"                        \
        ".global " #name ";"                         \
        ".equ " #name ", " #addr ";"                 \
        ".section .text;"                            \
        ".align 8, 0xCC;"                            \
        "_" #name ":;"                               \
        "push ebx;"                                  \
        "push ebp;"                                  \
        "mov ebp, esp;"                              \
        ".if " #arg_count " >= 16;"                  \
            ".error \"Too many args in macro\";"     \
        ".endif;"                                    \
        ".if " #arg_count " >= 15;"                  \
            "push [ebp+68];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 14;"                  \
            "push [ebp+64];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 13;"                  \
            "push [ebp+60];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 12;"                  \
            "push [ebp+56];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 11;"                  \
            "push [ebp+52];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 10;"                  \
            "push [ebp+48];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 9;"                   \
            "push [ebp+44];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 8;"                   \
            "push [ebp+40];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 7;"                   \
            "push [ebp+36];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 6;"                   \
            "push [ebp+32];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 5;"                   \
            "push [ebp+28];"                         \
        ".endif;"                                    \
        ".if " #arg_count " >= 4;"                   \
            "mov ecx, [ebp+24];"                     \
        ".endif;"                                    \
        ".if " #arg_count " >= 3;"                   \
            "mov ebx, [ebp+20];"                     \
        ".endif;"                                    \
        ".if " #arg_count " >= 2;"                   \
            "mov edx, [ebp+16];"                     \
        ".endif;"                                    \
        ".if " #arg_count " >= 1;"                   \
            "mov eax, [ebp+12];"                     \
        ".endif;"                                    \
        "call " #addr ";"                            \
        "pop ebp;"                                   \
        "pop ebx;"                                   \
        "ret;"                                       \
    )
