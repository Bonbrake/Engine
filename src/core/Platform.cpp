#include "ze/core/Platform.h"
#include "ze/core/Logger.h"

#ifdef _WIN32
#include <windows.h>
#include <minidumpapiset.h>
#pragma comment(lib, "dbghelp.lib")
#endif

#include <fstream>
#include <iostream>

namespace core {

void Platform::triggerBreakpoint() {
#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#endif
}

void Platform::writeMinidump(void* exceptionPointers) {
#ifdef _WIN32
    HANDLE hFile = CreateFileA("crash_dump.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION exInfo;
        exInfo.ThreadId = GetCurrentThreadId();
        exInfo.ExceptionPointers = (PEXCEPTION_POINTERS)exceptionPointers;
        exInfo.ClientPointers = FALSE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &exInfo, NULL, NULL);
        CloseHandle(hFile);
    }
#endif
}

void Platform::writeCrashSidecar(const std::string& info) {
    std::ofstream file("crash_sidecar.txt");
    if (file.is_open()) {
        file << "Crash occurred:\n" << info << "\n";
    }
}

#ifdef _WIN32
LONG WINAPI unhandledExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
    // Write minidump first (minimize deadlock risk)
    Platform::writeMinidump(exceptionInfo);
    
    // Write sidecar using raw file without dynamic formatting
    std::ofstream file("crash_sidecar.txt");
    if (file.is_open()) {
        file << "Crash occurred: Unhandled exception caught.\n";
    }
    
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void Platform::initCrashHandler() {
#ifdef _WIN32
    SetUnhandledExceptionFilter(unhandledExceptionHandler);
#endif
}

} // namespace core