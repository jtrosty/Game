@echo off
IF NOT EXIST w:\build\clang mkdir w:\build\clang
pushd w:\build\clang


set CommonCompilerFlags=-std=c++14 -g -D HANDMADE_INTERNAL -D HANDMADE_SLOW -D CLANG_COMPILE

set IncludeFlags= -I "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include" -L "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\lib\x64"
set LinkerFlags= -l "user32" -l "gdi32" -l "winmm" -l "kernel32" -D "_UNICODE" -D "UNICODE" 
set In= -I "C:\Program Files (x86)\WindowsKits\10\Include\10.0.19041.0\"

clang++ %CommonCompilerFlags% -c \src\core.cpp -o core_clang.dll -MJ clangd_dll.json -g %LinkerFlags% %In%
clang++ %CommonCompilerFlags% \src\win32_platform.cpp -o coreGame.exe -MJ clangd_final.json -g %LinkerFlags% %In%

popd


REM "C:\Program Files\LLVM\bin\clang++.exe" `
REM    -o .\bin\win32-program.exe `
REM    -I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\" `
REM    -L "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\" `
REM    -l "kernel32" `
REM    -l "user32" `
REM    -D "_UNICODE" `
REM    -D "UNICODE" `
REM    .\src\win32-program.cpp
