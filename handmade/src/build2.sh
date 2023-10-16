echo Building Handmade Hero

INCLUDED_FRAMEWORKS="-framework AppKit
                     -framework IOKit
                     -framework AudioToolbox"

RESOURCES_PATH="../resources"

BUNDLE_RESOURCES_PATH="handmade.app/Contents/Resources"

COMPILER_WARNING_FLAGS="-Werror 
                        -Weverything"

# NOTE: (TED) -Wpadded can be useful when we want to know which structs
#             are not properly padded. We have disabled it for now.

DISABLED_WARNINGS="-Wno-old-style-cast
                   -Wno-cast-qual
                   -Wno-gnu-anonymous-struct
                   -Wno-nested-anon-types
                   -Wno-padded
                   -Wno-unused-variable
                   -Wno-unused-parameter
                   -Wno-pedantic
                   -Wno-missing-prototypes
                   -Wno-deprecated-declarations
                   -Wno-nullable-to-nonnull-conversion
                   -Wno-c++11-long-long
                   -Wno-poison-system-directories"

                   

COMMON_COMPILER_FLAGS="$COMPILER_WARNING_FLAGS 
                       $DISABLED_WARNINGS 
                       -DHANDMADE_INTERNAL=1 
                       $INCLUDED_FRAMEWORKS"

mkdir build
pushd build
rm -rf handmade.app
mkdir -p $BUNDLE_RESOURCES_PATH
pwd
clang -g -o GameCode.dylib ${COMMON_COMPILER_FLAGS} -dynamiclib ../src/core.cpp
clang -g ${COMMON_COMPILER_FLAGS} -o handmade ../src/osx_main.mm
cp handmade handmade.app/handmade
cp "${RESOURCES_PATH}/Info.plist" handmade.app/Info.plist
cp "${RESOURCES_PATH}/test_background.bmp" $BUNDLE_RESOURCES_PATH
cp GameCode.dylib "${BUNDLE_RESOURCES_PATH}/GameCode.dylib"
cp -r GameCode.dylib.dSYM "${BUNDLE_RESOURCES_PATH}/GameCode.dylib.dSYM"
popd
