echo "building win32 platform"
@echo off
set PWD=%~dp0

REM set CommonCompilerFlags=-WL -Od -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4505 -wd4244 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7 -Gs9999999

set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4244 -wd4201 -wd4100 -wd4189 -wd4505 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib /DEBUG:FULL
set lib_ldtk = "..\external\libcLDtk.lib"

REM TODO - can we just build both with one exe?

IF NOT EXIST w:\build mkdir w:\build
pushd w:\build
REM pushd C:code\games\Game\handmade\build

REM 32-bit build
REM cl %CommonCompilerFlags% ..\handmade\code\win32_platform.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%


REM 64-bit build
del *.pdb > NUL 2> NUL
REM cl %InitialCompilerFlags% -o core ..\src\win32_platform.cpp -Fmwin32_platform.map /link  %CommonLinkerFlags%
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% w:\src\core.cpp -Fmcore.map -LD /link -incremental:no -opt:ref -PDB:core_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
del lock.tmp
cl %CommonCompilerFlags% w:\src\win32_platform.cpp -Fmwin32_platform.map /link %CommonLinkerFlags%
popd
echo "DONE building win32 platform"


REM Compiler Flags:

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
