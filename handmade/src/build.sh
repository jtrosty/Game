echo Building Handmade Hero

OSX_LD_FLAGS="-framework AppKit"

mkdir ../build
pushd ../build
clang -g $OSX_LD_FLAGS -o handmade ../src/osx_main.mm
popd