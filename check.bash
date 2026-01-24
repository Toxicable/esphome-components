#!/usr/bin/env bash
set -euo pipefail

rg --files --null \
  -g "*.c" \
  -g "*.cc" \
  -g "*.cpp" \
  -g "*.cxx" \
  -g "*.h" \
  -g "*.hh" \
  -g "*.hpp" \
  | xargs -0 -n 1 clangd --check
