#!/bin/bash
set -euo pipefail
project_dir="$(cd "$(dirname "$0")/.." && pwd)"
candidate="${1:-${OF_ROOT:-$project_dir/../../of_v0.12.1_osx_release}}"
if [ ! -f "$candidate/libs/openFrameworksCompiled/project/osx/CoreOF.xcconfig" ]; then
  echo "Provide the openFrameworks 0.12.1 macOS directory: ./scripts/configure.sh /path/to/openFrameworks" >&2
  exit 1
fi
of_root="$(cd "$candidate" && pwd)"
mkdir -p "$project_dir/.deps"
ln -sfn "$of_root" "$project_dir/.deps/openFrameworks"
echo "Using openFrameworks: $of_root"
