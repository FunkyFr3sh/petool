
#define SETINST(addr, inst)                         \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #addr ";"                          \
        ".long (2f - 1f);"                          \
        "1:;"                                       \
            inst ";"                                \
        "2:;"                                       \
        ".section .text;"                           \
    )

#define CLEAR(start, value, end)                    \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #start ";"                         \
        ".long " #end "-" #start ";"                \
        ".fill " #end "-" #start ", 1, " #value ";" \
        ".section .text;"                           \
    )

#define SJMP(src, dst)                              \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #src ";"                           \
        ".long 2;"                                  \
        ".byte 0xEB;"                               \
        ".byte " #dst "-" #src " - 2;"              \
        ".section .text;"                           \
    )

#define LJMP(src, dst)                              \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #src ";"                           \
        ".long 5;"                                  \
        ".byte 0xE9;"                               \
        ".long " #dst "-" #src " - 5;"              \
        ".section .text;"                           \
    )

#define SETDWORD(addr, value)                       \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #addr ";"                          \
        ".long 4;"                                  \
        ".long " #value ";"                         \
        ".section .text;"                           \
    )

#define SETWORD(addr, value)                        \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #addr ";"                          \
        ".long 2;"                                  \
        ".short " #value ";"                        \
        ".section .text;"                           \
    )

#define SETBYTE(addr, value)                        \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #addr ";"                          \
        ".long 1;"                                  \
        ".byte " #value ";"                         \
        ".section .text;"                           \
    )
    
#define SETBYTES(addr, value)                       \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #addr ";"                          \
        ".long (2f - 1f);"                          \
        "1:;"                                       \
            ".ascii " #value ";"                    \
        "2:;"                                       \
        ".section .text;"                           \
    )

#define HOOK_1(addr)                                \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #addr ";"                          \
        ".long 5;"                                  \
        ".byte 0xE9;"                               \
        ".long _dest" #addr "-" #addr " - 5;"       \
        ".section .text;"                           \
    );                                              \
    EXTERN_C void __attribute__((naked)) dest##addr()
    
#define HOOK_2(addr, end)                           \
    CLEAR_INT((addr + 5), end);                     \
    HOOK_1(addr)

#define HOOK_X(x,A,B,FUNC, ...)  FUNC  
#define HOOK(...)         HOOK_X(,##__VA_ARGS__,    \
                               HOOK_2(__VA_ARGS__), \
                               HOOK_1(__VA_ARGS__), \
                               HOOK_0(__VA_ARGS__)  \
                               )

#define CLEAR_NOP(start, end) CLEAR(start, 0x90, end)
#define CLEAR_INT(start, end) CLEAR(start, 0xCC, end)

#define LJMP_NOP(start, end, dst)                   \
    CLEAR_NOP((start + 5), end);                    \
    LJMP(start, dst)

#define LJMP_INT(start, end, dst)                   \
    CLEAR_INT((start + 5), end);                    \
    LJMP(start, dst)

#define CALL_2(src, dst)                            \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #src ";"                           \
        ".long 5;"                                  \
        ".byte 0xE8;"                               \
        ".long " #dst "-" #src " - 5;"              \
        ".section .text;"                           \
    )

#define CALL_3(src, dst, arg_count)                 \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #src ";"                           \
        ".long 5;"                                  \
        ".byte 0xE8;"                               \
        ".long 1f - " #src " - 5;"                  \
        ".section .text;"                           \
        ".align 8, 0xCC;"                           \
        ".if " #arg_count " == 0;"                  \
            "1:;"                                   \
            "push ecx;"                             \
            "push edx;"                             \
            "call " #dst ";"                        \
            "pop edx;"                              \
            "pop ecx;"                              \
            "ret;"                                  \
        ".elseif " #arg_count " == 1;"              \
            "1:;"                                   \
            "push ecx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 4;"                           \
            "pop edx;"                              \
            "pop ecx;"                              \
            "ret;"                                  \
        ".elseif " #arg_count " == 2;"              \
            "1:;"                                   \
            "push ecx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 8;"                           \
            "pop ecx;"                              \
            "ret;"                                  \
        ".elseif " #arg_count " == 3;"              \
            "1:;"                                   \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 12;"                          \
            "pop ecx;"                              \
            "ret;"                                  \
        ".elseif " #arg_count " == 4;"              \
            "1:;"                                   \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 16;"                          \
            "ret;"                                  \
        ".elseif " #arg_count " == 5;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 20;"                          \
            "pop ebp;"                              \
            "ret 4;"                                \
        ".elseif " #arg_count " == 6;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 24;"                          \
            "pop ebp;"                              \
            "ret 8;"                                \
        ".elseif " #arg_count " == 7;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+16];"                        \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 28;"                          \
            "pop ebp;"                              \
            "ret 12;"                               \
        ".elseif " #arg_count " == 8;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+20];"                        \
            "push [ebp+16];"                        \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 32;"                          \
            "pop ebp;"                              \
            "ret 16;"                               \
        ".elseif " #arg_count " == 9;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+24];"                        \
            "push [ebp+20];"                        \
            "push [ebp+16];"                        \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 36;"                          \
            "pop ebp;"                              \
            "ret 20;"                               \
        ".elseif " #arg_count " == 10;"             \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+28];"                        \
            "push [ebp+24];"                        \
            "push [ebp+20];"                        \
            "push [ebp+16];"                        \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 40;"                          \
            "pop ebp;"                              \
            "ret 24;"                               \
        ".endif;"                                   \
    )

#define CALL_X(x,A,B,C,FUNC, ...)  FUNC  
#define CALL(...)                                   \
                          CALL_X(,##__VA_ARGS__,    \
                               CALL_3(__VA_ARGS__), \
                               CALL_2(__VA_ARGS__), \
                               CALL_1(__VA_ARGS__), \
                               CALL_0(__VA_ARGS__)  \
                               )

#define CALL_NOP_2(src, dst)                        \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #src ";"                           \
        ".long 6;"                                  \
        ".byte 0xE8;"                               \
        ".long " #dst "-" #src " - 5;"              \
        ".byte 0x90;"                               \
        ".section .text;"                           \
    )

#define CALL_NOP_3(src, dst, arg_count)             \
    __asm (                                         \
        ".section .patch,\"d0\";"                   \
        ".long " #src ";"                           \
        ".long 6;"                                  \
        ".byte 0xE8;"                               \
        ".long 1f - " #src " - 5;"                  \
        ".byte 0x90;"                               \
        ".section .text;"                           \
        ".align 8, 0xCC;"                           \
        ".if " #arg_count " == 0;"                  \
            "1:;"                                   \
            "push ecx;"                             \
            "push edx;"                             \
            "call " #dst ";"                        \
            "pop edx;"                              \
            "pop ecx;"                              \
            "ret;"                                  \
        ".elseif " #arg_count " == 1;"              \
            "1:;"                                   \
            "push ecx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 4;"                           \
            "pop edx;"                              \
            "pop ecx;"                              \
            "ret;"                                  \
        ".elseif " #arg_count " == 2;"              \
            "1:;"                                   \
            "push ecx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 8;"                           \
            "pop ecx;"                              \
            "ret;"                                  \
        ".elseif " #arg_count " == 3;"              \
            "1:;"                                   \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 12;"                          \
            "pop ecx;"                              \
            "ret;"                                  \
        ".elseif " #arg_count " == 4;"              \
            "1:;"                                   \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 16;"                          \
            "ret;"                                  \
        ".elseif " #arg_count " == 5;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 20;"                          \
            "pop ebp;"                              \
            "ret 4;"                                \
        ".elseif " #arg_count " == 6;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 24;"                          \
            "pop ebp;"                              \
            "ret 8;"                                \
        ".elseif " #arg_count " == 7;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+16];"                        \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 28;"                          \
            "pop ebp;"                              \
            "ret 12;"                               \
        ".elseif " #arg_count " == 8;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+20];"                        \
            "push [ebp+16];"                        \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 32;"                          \
            "pop ebp;"                              \
            "ret 16;"                               \
        ".elseif " #arg_count " == 9;"              \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+24];"                        \
            "push [ebp+20];"                        \
            "push [ebp+16];"                        \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 36;"                          \
            "pop ebp;"                              \
            "ret 20;"                               \
        ".elseif " #arg_count " == 10;"             \
            "1:;"                                   \
            "push ebp;"                             \
            "mov ebp, esp;"                         \
            "push [ebp+28];"                        \
            "push [ebp+24];"                        \
            "push [ebp+20];"                        \
            "push [ebp+16];"                        \
            "push [ebp+12];"                        \
            "push [ebp+8];"                         \
            "push ecx;"                             \
            "push ebx;"                             \
            "push edx;"                             \
            "push eax;"                             \
            "call " #dst ";"                        \
            "add esp, 40;"                          \
            "pop ebp;"                              \
            "ret 24;"                               \
        ".endif;"                                   \
    )

#define CALL_NOP_X(x,A,B,C,FUNC, ...)  FUNC  
#define CALL_NOP(...)                               \
                      CALL_NOP_X(,##__VA_ARGS__,    \
                           CALL_NOP_3(__VA_ARGS__), \
                           CALL_NOP_2(__VA_ARGS__), \
                           CALL_NOP_1(__VA_ARGS__), \
                           CALL_NOP_0(__VA_ARGS__)  \
                           )
