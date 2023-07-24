
# set CommonCompilerFlags=-WL -Od -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4505 -wd4244 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7 -Gs9999999

CommonCompilerFlags="-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4244 -wd4201 -wd4100 -wd4189 -wd4505 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7"
CommonLinkerFlags= "-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib"

# TODO - can we just build both with one exe?

IF NOT EXIST build mkdir build
pushd build

# 32-bit build
# cl.exe %CommonCompilerFlags% ..\handmade\code\win32_platform.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

# 64-bit build
del *.pdb > NUL 2> NUL
# cl.exe %InitialCompilerFlags% -o core ..\src\win32_platform.cpp -Fmwin32_platform.map /link  %CommonLinkerFlags%
echo WAITING FOR PDB > lock.tmp
cl.exe $CommonCompilerFlags ../src/core.cpp -Fmcore.map -LD \link -incremental:no -opt:ref -PDB:core_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
del lock.tmp
cl.exe $CommonCompilerFlags ../src/win32_platform.cpp -Fmwin32_platform.map /link $CommonLinkerFlags
popd


# Compiler Flags:

# Zi  : debug info (Z7 older debug format for complex builds)
# Zo  : More debug info for optimized builds
# FC  : Full path on errors
# Oi  : Always do intrinsics with you can
# Od  : No optimizations
# O2  : Full optimizations
# MT  : Use the c static lib instead of searching for dll at run-time
# MTd : Sabe as MT but using the debug version of CRT
# GR- : Turn off C++ run-time type info
# Gm- : Turn off incremental build
# EHa-: Turn off exception handling
# WX  : Treat warning as errors
# W4  : Set Warning level to 4 (Wall to all levels)
# wd  : Ignore warning
# fp:fast    : Ignores the rules in some cases to optimize fp operations
# Fmfile.map : Outputs a map file (mapping of the functions on the exr)

# Linker Options:

# subsystem:windows,5.1 : Make exe compatible with Windows XP (only works on x86)
# opt:ref               : Don't put unused things in the exe
# incremental:no        : Don't need to do incremental builds
# LD                    : Build a dll
# PDB:file.pdb          : Change the .pdb's path
