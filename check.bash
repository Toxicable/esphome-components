#!/usr/bin/env bash
set -euo pipefail

filter_cc1_args() {
  awk '
    /internal \(cc1\) args are:/ {next}
    /Generic fallback command is:/ {next}
    {print}
  '
}
export -f filter_cc1_args

if [[ $# -gt 0 ]]; then
  clangd --check="$1" 2>&1 | filter_cc1_args
  exit 0
fi

rg --files --null \
  -g "*.c" \
  -g "*.cc" \
  -g "*.cpp" \
  -g "*.cxx" \
  -g "*.h" \
  -g "*.hh" \
  -g "*.hpp" \
  | xargs -0 -n 1 -I{} bash -c 'clangd --check="$1" 2>&1 | filter_cc1_args' _ {}
