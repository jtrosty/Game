echo Building Trost Cake

OSX_FRAMEWORKS="-framework AppKit -framework Metal -framework Foundation -framework Cocoa -framework CoreGraphics -framework MetalKit"
OSX_FLAGS="-Wall -std=c++17 -I./metal-cpp -I./metal-cpp-extensions -fno-objc-arc"

APP_NAME="Trolib"
APP_BUNDLE_NAME="Trost.app"

# pushd moves us into a directory where we will exectue commands
mkdir buildOSX
pushd buildOSX

# This is the mac platform layer path from inside of the build directory for mac.
pwd
MAC_PLATFORM_LAYER_PATH=../src

# clang will be invoked from inside of the build directory
clang -g $OSX_FLAGS $OSX_FRAMEWORKS -o $APP_NAME ${MAC_PLATFORM_LAYER_PATH}/osx_main.mm

rm -rf $APP_BUNDLE_NAME
mkdir -p ${APP_BUNDLE_NAME}/Contents

#cp $APP_NAME ${APP_BUNDLE_NAME}/${APP_NAME}
#cp ${MAC_PLATFORM_LAYER_PATH}/resources/Info.plist ${APP_BUNDLE_NAME}/Contents/Info.plist

popd

echo Done Building Trost Cake
