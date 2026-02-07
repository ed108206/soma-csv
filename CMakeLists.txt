cmake_minimum_required(VERSION 3.15)
project(Soma LANGUAGES CXX RC)

set(SOURCES
    SplitterE.cpp
    Soma.cpp
    EView.cpp
    LView.cpp
    FrameProc.cpp
    ExtraTree.cpp
    ExtraFrame.cpp
    Soma.rc
)

set(COMMON_CFLAGS "-m64 -mavx2 -march=native")

# --- Release ---
set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_CFLAGS} -O3 -Wall -fexceptions -Wexpansion-to-defined -Wcast-function-type -DNDEBUG -D_WINDOWS -D_MBCS -s")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-m64 -mwindows")

# --- Debug ---
set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_CFLAGS} -Wall -fexceptions -g -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-m64 -mwindows")

add_executable(Soma ${SOURCES})

# AVX2
target_compile_options(Soma PRIVATE -m64 -mavx2 -march=native)

set_target_properties(Soma PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
    OUTPUT_NAME_DEBUG   "dSoma"
    OUTPUT_NAME_RELEASE "Soma"
)

target_link_libraries(Soma
    comctl32
    gdi32
    comdlg32
    pthread
)

target_link_libraries(Soma
    $<$<CONFIG:Release>:kernel32 user32 gdi32 comdlg32 advapi32 shell32 ole32 oleaut32 uuid>
    $<$<CONFIG:Debug>:kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32>
)