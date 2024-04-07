#ifndef PATCH_H 
#define PATCH_H 

#include <windows.h>

static inline void patch_clear(char *start, char value, char *end)
{
    DWORD op = PAGE_EXECUTE_READ;
    VirtualProtect(start, end - start, PAGE_EXECUTE_READWRITE, &op);
    memset(start, value, end - start);
    VirtualProtect(start, end - start, op, &op);
}

static inline void patch_sjmp(char *src, char *dst)
{
    DWORD op = PAGE_EXECUTE_READ;
    VirtualProtect(src, 2, PAGE_EXECUTE_READWRITE, &op);
    src[0] = '\xEB';
    src[1] = dst - src - 2;
    VirtualProtect(src, 2, op, &op);
}

static inline void patch_ljmp(char *src, char *dst)
{
    DWORD op = PAGE_EXECUTE_READ;
    VirtualProtect(src, 5, PAGE_EXECUTE_READWRITE, &op);
    src[0] = '\xE9';
    *((DWORD *)(&src[1])) = dst - src - 5;
    VirtualProtect(src, 5, op, &op);
}

static inline PROC patch_call(char *src, char *dst)
{
    DWORD op = PAGE_EXECUTE_READ;
    VirtualProtect(src, 5, PAGE_EXECUTE_READWRITE, &op);
    src[0] = '\xE8';
    DWORD org = *((DWORD *)(&src[1]));
    *((DWORD *)(&src[1])) = dst - src - 5;
    VirtualProtect(src, 5, op, &op);
    return (PROC)(src + 5 + org);
}

static inline void patch_call_nop(char *src, char *dst)
{
    DWORD op = PAGE_EXECUTE_READ;
    VirtualProtect(src, 6, PAGE_EXECUTE_READWRITE, &op);
    src[0] = '\xE8';
    *((DWORD *)(&src[1])) = dst - src - 5;
    src[5] = '\x90';
    VirtualProtect(src, 6, op, &op);
}

static inline DWORD patch_setdword(DWORD *dst, DWORD value)
{
    DWORD op = PAGE_EXECUTE_READ;
    VirtualProtect(dst, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &op);
    DWORD org = *dst;
    *dst = value; 
    VirtualProtect(dst, sizeof(DWORD), op, &op);
    return org;
}

static inline WORD patch_setword(WORD *dst, WORD value)
{
    DWORD op = PAGE_EXECUTE_READ;
    VirtualProtect(dst, sizeof(WORD), PAGE_EXECUTE_READWRITE, &op);
    WORD org = *dst;
    *dst = value; 
    VirtualProtect(dst, sizeof(WORD), op, &op);
    return org;
}

static inline BYTE patch_setbyte(BYTE *dst, BYTE value)
{
    DWORD op = PAGE_EXECUTE_READ;
    VirtualProtect(dst, sizeof(BYTE), PAGE_EXECUTE_READWRITE, &op);
    BYTE org = *dst;
    *dst = value; 
    VirtualProtect(dst, sizeof(BYTE), op, &op);
    return org;
}

static inline void patch_setbytes(char *dst, char *buf, size_t size)
{
    DWORD op = PAGE_EXECUTE_READ;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &op);
    memcpy(dst, buf, size);
    VirtualProtect(dst, size, op, &op);
}

#define PATCH_SET(a,b) patch_setbytes(a,b,sizeof(b)-1)

static inline void patch_clear_nop(char *start, char *end)
{
    return patch_clear(start, '\x90', end);
}

static inline void patch_clear_int(char *start, char *end)
{
    return patch_clear(start, '\xCC', end);
}

#endif
