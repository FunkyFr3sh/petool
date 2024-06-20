%include "macros/patch.inc"
%include "macros/datatypes.inc"

gfunction watcall
%define i ebp-4
%define destFunction ebp-8
%define args ebp-52
%define valist ebp-56

    pop eax
    pop ecx
    push ebp
    push ebx
    mov ebp, esp
    sub esp, 64
    mov dword[i], eax ; argCount
    mov dword[destFunction], ecx
    lea eax, [eax*4+ebp+8]
    mov dword[valist], eax
    inc dword[i]
    jmp .start
    
.loop:
    mov eax, dword[valist]
    lea edx, [eax-4]
    mov dword[valist], edx
    mov edx, dword[eax]
    mov eax, dword[i]
    cmp eax, 4
    jbe .noPush
    push edx
    jmp .start
    
.noPush:
    mov dword[eax*4+args], edx
    
.start:
    dec dword[i]
    jnz .loop

    mov ecx, dword[args+4+4+4+4]
    mov ebx, dword[args+4+4+4]
    mov edx, dword[args+4+4]
    mov eax, dword[args+4]
    call [destFunction]
    
    mov esp, ebp
    pop ebx
    pop ebp
    retn
