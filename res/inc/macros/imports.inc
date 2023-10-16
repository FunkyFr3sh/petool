%ifndef IMPORTS_INC
%define IMPORTS_INC

%macro import_library 5
    %defstr ___TmpStringX_ %1

%ifndef str_%1_dll_1
%define str_%1_dll_1 1
    [section .rdata]
    str_%1_dll db ___TmpStringX_,0,0
    __SECT__
%endif

    [section .idata]
    dd (%2)                         ; OriginalFirstThunk
    dd (%3)                         ; TimeDateStamp
    dd (%4)                         ; ForwarderChain
    dd str_%1_dll - ImageBase       ; Name
    dd (%5)                         ; FirstThunk
    __SECT__
%endmacro

%macro import_library 1
    import_library %1, %1_oft_ - ImageBase, 0x0, 0x0, %1_ft_ - ImageBase
%endmacro

%macro import_library_end 0
    [section .idata]
    dd 0x0                          ; OriginalFirstThunk
    dd 0x0                          ; TimeDateStamp
    dd 0x0                          ; ForwarderChain
    dd 0x0                          ; Name
    dd 0x0                          ; FirstThunk
    __SECT__
%endmacro


%macro import_function 2
    %defstr ___TmpStringX_ %2
   
    ; AddressOfData
    [section .rdata]
    %2_aod_:                  
    dw 0
    db ___TmpStringX_,0,0
    __SECT__
    
    ; OriginalFirstThunk
    [section .data]
%ifndef %1_oft_1
%define %1_oft_1 1
    align 4, db 0x00
    %1_oft_:                  
%endif
    dd %2_aod_ - ImageBase
    __SECT__
    
    ; FirstThunk
    global __imp__%2
    [section .idata]
%ifndef %1_ft_1
%define %1_ft_1 1
    align 4, db 0x00
    %1_ft_:                   
%endif
    __imp__%2 dd %2_aod_ - ImageBase
    __SECT__
    
    ; Jump table
    global _%2
    [section .text]
    align 4, db 0xCC
    _%2:
        jmp [__imp__%2]
    __SECT__
%endmacro

%macro import_function_end 0
    [section .data]
    dd 0
    __SECT__
    
    [section .idata]
    dd 0  
    __SECT__
%endmacro

%endif
