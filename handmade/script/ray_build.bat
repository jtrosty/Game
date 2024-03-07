
@echo off
IF NOT EXIST w:\build\clang mkdir w:\build\clang
pushd w:\build\clang


set CommonCompilerFlags=-std=c++14 -g -D HANDMADE_INTERNAL -D HANDMADE_SLOW -D WIN_RAY

set IncludeRayFlags= -I "C:\raylib\raylib\src\" -L "C:\raylib\raylib\src\libraylib.a"
set LinkerFlags= -l "lraylib" -l "lkernel32" -l "lopengl32" -l "lwinmm" -l "lgdi32" "-mwindows"
REM set LinkerFlags= "lraylib -lopengl -lgdi32 -lwinmm -Wall -mwindows"

REM set In= -I "C:\Program Files (x86)\WindowsKits\10\Include\10.0.19041.0\"

clang++ %CommonCompilerFlags% -c w:\src\core.cpp -o ray_core.dll -MJ clangd_dll.json -g %LinkerFlags% %In%
clang++ %CommonCompilerFlags% w:\src\ray\ray_platform.cpp -o ray_CoreGame.exe -MJ ray_clangd_final.json -g %LinkerFlags% %IncludeRayFlags% %In% 


REM Compiler Flags:
REM

REM Zi  : debug info (Z7 older debug format for complex builds)
REM Zo  : More debug info for optimized builds
REM FC  : Full path on errors
REM Oi  : Always do intrinsics with you can
REM Od  : No optimizations
REM O2  : Full optimizations
REM MT  : Use the c static lib instead of searching for dll at run-time
REM MTd : Sabe as MT but using the debug version of CRT
REM GR- : Turn off C++ run-time type info
REM Gm- : Turn off incremental build
REM EHa-: Turn off exception handling
REM WX  : Treat warning as errors
REM W4  : Set Warning level to 4 (Wall to all levels)
REM wd  : Ignore warning
REM fp:fast    : Ignores the rules in some cases to optimize fp operations
REM Fmfile.map : Outputs a map file (mapping of the functions on the exr)

REM Linker Options:

REM subsystem:windows,5.1 : Make exe compatible with Windows XP (only works on x86)
REM opt:ref               : Don't put unused things in the exe
REM incremental:no        : Don't need to do incremental builds
REM LD                    : Build a dll
REM PDB:file.pdb          : Change the .pdb's path
