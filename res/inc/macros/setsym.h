
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
        "_" #name ":;"                               \
        "push " #addr ";"                            \
        "push " #arg_count ";"                       \
        "jmp _watcall;"                              \
    )
