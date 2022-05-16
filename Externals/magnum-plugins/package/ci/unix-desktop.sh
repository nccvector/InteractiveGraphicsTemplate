#!/bin/bash
set -ev

# Corrade
git clone --depth 1 https://github.com/mosra/corrade.git
cd corrade
mkdir build && cd build
cmake .. \
    -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCMAKE_INSTALL_RPATH=$HOME/deps/lib \
    -DCMAKE_BUILD_TYPE=$CONFIGURATION \
    -DWITH_INTERCONNECT=OFF \
    -DBUILD_DEPRECATED=$BUILD_DEPRECATED \
    -DBUILD_STATIC=$BUILD_STATIC \
    -G Ninja
ninja install
cd ../..

# Magnum
git clone --depth 1 https://github.com/mosra/magnum.git
cd magnum
mkdir build && cd build
cmake .. \
    -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCMAKE_INSTALL_RPATH=$HOME/deps/lib \
    -DCMAKE_BUILD_TYPE=$CONFIGURATION \
    -DWITH_AUDIO=ON \
    -DWITH_DEBUGTOOLS=ON \
    -DWITH_GL=OFF \
    -DWITH_MESHTOOLS=ON \
    -DWITH_PRIMITIVES=ON \
    -DWITH_SCENEGRAPH=OFF \
    -DWITH_SCENETOOLS=OFF \
    -DWITH_SHADERS=OFF \
    -DWITH_TEXT=ON \
    -DWITH_TEXTURETOOLS=ON \
    -DWITH_ANYIMAGEIMPORTER=ON \
    -DBUILD_DEPRECATED=$BUILD_DEPRECATED \
    -DBUILD_STATIC=$BUILD_STATIC \
    -DBUILD_PLUGINS_STATIC=$BUILD_STATIC \
    -G Ninja
ninja install
cd ../..

# Build. BUILD_GL_TESTS is enabled just to be sure, it should not be needed by
# any plugin.
mkdir build && cd build
cmake .. \
    -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" \
    -DCMAKE_INSTALL_PREFIX=$HOME/deps \
    -DCMAKE_INSTALL_RPATH=$HOME/deps/lib \
    -DCMAKE_BUILD_TYPE=$CONFIGURATION \
    -DWITH_ASSIMPIMPORTER=ON \
    -DWITH_BASISIMAGECONVERTER=ON \
    -DWITH_BASISIMPORTER=ON -DBASIS_UNIVERSAL_DIR=$HOME/basis_universal \
    -DWITH_CGLTFIMPORTER=$BUILD_DEPRECATED \
    -DWITH_DDSIMPORTER=ON \
    -DWITH_DEVILIMAGEIMPORTER=ON \
    -DWITH_DRFLACAUDIOIMPORTER=ON \
    -DWITH_DRMP3AUDIOIMPORTER=ON \
    -DWITH_DRWAVAUDIOIMPORTER=ON \
    $FREETYPE_INCLUDE_DIR_freetype2 \
    -DWITH_FAAD2AUDIOIMPORTER=ON \
    -DWITH_FREETYPEFONT=ON \
    -DWITH_GLSLANGSHADERCONVERTER=ON \
    -DWITH_GLTFIMPORTER=ON \
    -DWITH_HARFBUZZFONT=ON \
    -DWITH_ICOIMPORTER=ON \
    -DWITH_JPEGIMAGECONVERTER=ON \
    -DWITH_JPEGIMPORTER=ON \
    -DWITH_KTXIMAGECONVERTER=ON \
    -DWITH_KTXIMPORTER=ON \
    -DWITH_MESHOPTIMIZERSCENECONVERTER=ON \
    -DWITH_MINIEXRIMAGECONVERTER=ON \
    -DWITH_OPENEXRIMAGECONVERTER=ON \
    -DWITH_OPENEXRIMPORTER=ON \
    -DWITH_OPENGEXIMPORTER=ON \
    -DWITH_PNGIMAGECONVERTER=ON \
    -DWITH_PNGIMPORTER=ON \
    -DWITH_PRIMITIVEIMPORTER=ON \
    -DWITH_SPIRVTOOLSSHADERCONVERTER=ON \
    -DWITH_STANFORDIMPORTER=ON \
    -DWITH_STANFORDSCENECONVERTER=ON \
    -DWITH_STBDXTIMAGECONVERTER=ON \
    -DWITH_STBIMAGECONVERTER=ON \
    -DWITH_STBIMAGEIMPORTER=ON \
    -DWITH_STBTRUETYPEFONT=ON \
    -DWITH_STBVORBISAUDIOIMPORTER=ON \
    -DWITH_STLIMPORTER=ON \
    -DWITH_TINYGLTFIMPORTER=$BUILD_DEPRECATED \
    -DBUILD_TESTS=ON \
    -DBUILD_GL_TESTS=ON \
    -DBUILD_STATIC=$BUILD_STATIC \
    -DBUILD_PLUGINS_STATIC=$BUILD_STATIC \
    -G Ninja
ninja $NINJA_JOBS

# DevIL tests "leak" since testing directly the dlopen()ed dynamic plugin, was
# not a problem when testing a statically built library.
ASAN_OPTIONS="color=always" LSAN_OPTIONS="color=always suppressions=$(pwd)/../package/ci/leaksanitizer.conf" TSAN_OPTIONS="color=always" CORRADE_TEST_COLOR=ON ctest -V -E "DevIl"
ASAN_OPTIONS="color=always" LSAN_OPTIONS="color=always suppressions=$(pwd)/../package/ci/leaksanitizer-devil.conf" TSAN_OPTIONS="color=always" CORRADE_TEST_COLOR=ON ctest -V -R DevIl

# Test install, after running the tests as for them it shouldn't be needed
ninja install