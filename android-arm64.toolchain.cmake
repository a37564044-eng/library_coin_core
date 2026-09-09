set(CMAKE_SYSTEM_NAME Generic)

set(ANDROID_NDK_ROOT "$ENV{HOME}/android-build/ndk-arm64/android-ndk-r29")
set(LLVM "${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-arm64")

set(CMAKE_C_COMPILER "${LLVM}/bin/aarch64-linux-android34-clang")
set(CMAKE_CXX_COMPILER "${LLVM}/bin/aarch64-linux-android34-clang++")

set(CMAKE_AR "${LLVM}/bin/llvm-ar")
set(CMAKE_RANLIB "${LLVM}/bin/llvm-ranlib")
set(CMAKE_STRIP "${LLVM}/bin/llvm-strip")

set(CMAKE_C_FLAGS "--target=aarch64-linux-android34")
set(CMAKE_CXX_FLAGS "--target=aarch64-linux-android34")

set(CMAKE_EXE_LINKER_FLAGS "--target=aarch64-linux-android34")
set(CMAKE_SHARED_LINKER_FLAGS "--target=aarch64-linux-android34")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
