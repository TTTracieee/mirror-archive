#!/bin/bash
set -euo pipefail
project_dir="$(cd "$(dirname "$0")/.." && pwd)"
of_root="${OF_ROOT:-$project_dir/.deps/openFrameworks}"
if [ ! -d "$of_root/libs" ]; then of_root="$project_dir/../../of_v0.12.1_osx_release"; fi
opencv="$of_root/addons/ofxOpenCv/libs/opencv/lib/macos/opencv.xcframework/macos-arm64_x86_64"
test_dir="$(mktemp -d /private/tmp/mirror-tests.XXXXXX)"
trap 'rm -rf "$test_dir"' EXIT
xcrun clang++ -std=c++17 -O2 -arch "$(uname -m)" -I"$opencv/Headers" -I"$project_dir/src" \
  "$project_dir/tests/VisionTests.cpp" "$opencv/opencv.a" \
  "$of_root/libs/libpng/lib/macos/libpng.xcframework/macos-arm64_x86_64/libpng.a" \
  "$of_root/libs/FreeImage/lib/macos/FreeImage.xcframework/macos-arm64_x86_64/FreeImage.a" \
  -framework Accelerate -framework Cocoa -framework AVFoundation -framework CoreMedia \
  -framework CoreVideo -framework OpenCL -lz -o "$test_dir/vision_tests"
"$test_dir/vision_tests"
