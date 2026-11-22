#!/bin/bash
set -euo pipefail
project_dir="$(cd "$(dirname "$0")/.." && pwd)"
python3 -m venv "$project_dir/.venv"
"$project_dir/.venv/bin/python" -m pip install -r "$project_dir/requirements-camera.txt"
"$project_dir/.venv/bin/python" "$project_dir/bin/data/hand/tracker.py" --model "$project_dir/bin/data/hand/hand_landmarker.task" --probe
