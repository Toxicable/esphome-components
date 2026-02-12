#!/usr/bin/env bash
set -euo pipefail

if [[ $# -gt 0 ]]; then
  python tools/check_py_compile.py "$@"
  exit 0
fi

mapfile -d '' files < <(rg --files --null -g "*.py")

if [[ ${#files[@]} -eq 0 ]]; then
  exit 0
fi

python tools/check_py_compile.py "${files[@]}"
