set CommonCompilerFlags=-MTd -nologo -fp:fast -GR- -EHa- -Od -Oi -WX -W4 -wd4244 -wd4201 -wd4100 -wd4189 -wd4505 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

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
clang-cl %CommonCompilerFlags% w:\src\core.cpp -LD /link -incremental:no -opt:ref -PDB:core_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
del lock.tmp
clang-cl %CommonCompilerFlags% w:\src\win32_platform.cpp /link %CommonLinkerFlags%
popd
