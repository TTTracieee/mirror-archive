#!/bin/bash
set -euo pipefail
project_dir="$(cd "$(dirname "$0")/.." && pwd)"
if [ ! -d "$project_dir/.deps/openFrameworks/libs" ]; then "$project_dir/scripts/configure.sh"; fi
xcodebuild -quiet -project "$project_dir/MirrorArchive.xcodeproj" -scheme "MirrorArchive Release" \
  -configuration Release -arch "$(uname -m)" -derivedDataPath "$project_dir/build" \
  CODE_SIGN_IDENTITY=- CODE_SIGNING_REQUIRED=NO
echo "Built: $project_dir/build/Build/Products/Release/MirrorArchive.app"
