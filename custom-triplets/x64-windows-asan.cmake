set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CXX_COMPILER "C:/Program Files/LLVM/bin/clang-cl.exe")
set(VCPKG_C_COMPILER "C:/Program Files/LLVM/bin/clang-cl.exe")

# Inject ASan flags.
set(VCPKG_C_FLAGS "-fsanitize=address -D_DISABLE_STRING_ANNOTATION -D_DISABLE_VECTOR_ANNOTATION")
set(VCPKG_CXX_FLAGS "-fsanitize=address -D_DISABLE_STRING_ANNOTATION -D_DISABLE_VECTOR_ANNOTATION")
set(VCPKG_LINKER_FLAGS "-libpath:C:/PROGRA~1/LLVM/lib/clang/22/lib/windows clang_rt.asan_dynamic-x86_64.lib clang_rt.asan_dynamic_runtime_thunk-x86_64.lib")


# Prevent MSVC debug runtime from being used, because ASan doesn't support it on Windows.
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
    "-DCMAKE_C_COMPILER=C:/Program Files/LLVM/bin/clang-cl.exe"
    "-DCMAKE_CXX_COMPILER=C:/Program Files/LLVM/bin/clang-cl.exe"
    "-DCMAKE_RC_FLAGS="
)

# For ports that don't use CMake, override the debug flags to use /MD instead of /MDd
# We also must replace /D_DEBUG with /DNDEBUG because MSVC headers will expect debug CRT functions if _DEBUG is defined, but ASan forces the release CRT (/MD).
set(VCPKG_C_FLAGS_DEBUG "/DNDEBUG /Zi /Ob0 /Od /RTC1 /MD")
set(VCPKG_CXX_FLAGS_DEBUG "/DNDEBUG /Zi /Ob0 /Od /RTC1 /MD")
set(VCPKG_LINKER_FLAGS_DEBUG "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
set(VCPKG_CMAKE_CONFIGURE_OPTIONS_DEBUG "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL")
