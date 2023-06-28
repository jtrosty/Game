echo Building Trost Cake

OSX_LD_FLAGS="-framework AppKit"

mkdir buildOSX
pushd buildOSX
clang -g $OSX_LD_FLAGS -o platform ../src/osx_main.mm
popd