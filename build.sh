#!/usr/bin/env bash
set -e
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT:-./vcpkg}/scripts/buildsystems/vcpkg.cmake"
cmake --build build --parallel $(nproc)
echo "✅ 构建完成"
