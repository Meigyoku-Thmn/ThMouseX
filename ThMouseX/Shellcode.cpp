#include "Shellcode.h"
#include "../Common/Helper.h"

namespace helper = common::helper;

namespace core::shellcode {
    SIZE_T ShellcodeSectionSize;
    ON_INIT{
        auto& nt_header = *(PIMAGE_NT_HEADERS)((DWORD)&__ImageBase + __ImageBase.e_lfanew);
        auto sectionHeaders = (PIMAGE_SECTION_HEADER)(
            (DWORD)&nt_header +
            sizeof(nt_header.Signature) +
            (DWORD)(sizeof(nt_header.FileHeader)) +
            (DWORD)nt_header.FileHeader.SizeOfOptionalHeader
        );
        for (size_t i = 0; i < nt_header.FileHeader.NumberOfSections; i++) {
            auto& sectionHeader = sectionHeaders[i];
            if (strncmp((PCHAR)sectionHeader.Name, SHELLCODE_SECTION_NAME, ARRAYSIZE(sectionHeader.Name)) != 0)
                continue;
            ShellcodeSectionSize = sectionHeader.Misc.VirtualSize;
            break;
        }
    };
    // this only works with /JMC (Just My Code) disabled
#pragma runtime_checks("", off)
    SHELLCODE DWORD WINAPI UnloadingShellcode(ShellcodeInput* inp) {
        UNICODE_STRING user32dll{};
        inp->RtlInitUnicodeString(&user32dll, inp->user32dll);
        HMODULE user32{};
        inp->LdrLoadDll(nil, 0, &user32dll, &user32);
        ANSI_STRING peekMessageW{};
        inp->RtlInitAnsiString(&peekMessageW, inp->peekMessageW);
        decltype(&PeekMessageW) PeekMessageW{};
        inp->LdrGetProcedureAddress(user32, &peekMessageW, 0, (PVOID*)&PeekMessageW);
        MSG msg;
        PeekMessageW(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
        inp->LdrUnloadDll(user32);
        return 0;
    }
#pragma runtime_checks("", restore)
}