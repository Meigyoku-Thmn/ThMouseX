#include "Shellcode.h"
#include "../Common/Helper.h"

#include <cstdint>

namespace helper = common::helper;

namespace core::shellcode {
    SIZE_T ShellcodeSectionSize;
    void Initialize() {
        auto& nt_header = *bcast<PIMAGE_NT_HEADERS>(bcast<uintptr_t>(&__ImageBase) + __ImageBase.e_lfanew);
        auto sectionHeaders = bcast<PIMAGE_SECTION_HEADER>(
            bcast<uintptr_t>(&nt_header) +
            sizeof(nt_header.Signature) +
            sizeof(nt_header.FileHeader) +
            nt_header.FileHeader.SizeOfOptionalHeader
            );
        for (size_t i = 0; i < nt_header.FileHeader.NumberOfSections; i++) {
            auto& sectionHeader = sectionHeaders[i];
            if (strncmp(rcast<PCHAR>(sectionHeader.Name), SHELLCODE_SECTION_NAME, ARRAYSIZE(sectionHeader.Name)) != 0)
                continue;
            ShellcodeSectionSize = sectionHeader.Misc.VirtualSize;
            break;
        }
    }
    // this only works with /JMC (Just My Code) disabled
#pragma runtime_checks("", off)
    SHELLCODE DWORD WINAPI UnloadingShellcode(const ShellcodeInput* inp) {
        UNICODE_STRING user32dll{};
        inp->_RtlInitUnicodeString(&user32dll, inp->user32dll);
        HMODULE user32{};
        inp->_LdrLoadDll(nil, 0, &user32dll, &user32);
        ANSI_STRING peekMessageW{};
        inp->_RtlInitAnsiString(&peekMessageW, inp->peekMessageW);
        PVOID peekMessageWFunc{};
        inp->_LdrGetProcedureAddress(user32, &peekMessageW, 0, &peekMessageWFunc);
        MSG msg;
        bcast<decltype(&PeekMessageW)>(peekMessageWFunc)(&msg, nil, WM_USER, WM_USER, PM_NOREMOVE);
        inp->_LdrUnloadDll(user32);
        return 0;
    }
#pragma runtime_checks("", restore)
}