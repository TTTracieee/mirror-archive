#!/bin/bash
set -euo pipefail
report_dir="$(cd "$(dirname "$0")" && pwd)"
cd "$report_dir"
tectonic MiniProject_Report.tex
