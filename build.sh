#!/usr/bin/env bash
set -e
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 1. 初始化子模块
git submodule update --init --recursive

# 2. 安装系统依赖（如未安装）
command -v ninja >/dev/null 2>&1 || sudo apt-get install -y clang ninja-build cmake pkg-config

# 3. vcpkg
[[ -d vcpkg ]] || git clone https://github.com/microsoft/vcpkg.git vcpkg
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install fmt gtest --triplet=x64-linux

# 4. 构建 mini-stl 并跑测试
cmake -S mini-stl -B mini-stl/build \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$ROOT_DIR/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build mini-stl/build --parallel
cd mini-stl/build
ctest --output-on-failure --output-junit test_result.xml