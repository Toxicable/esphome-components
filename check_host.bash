#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$repo_root"

python3 tools/check_core_purity.py components/component_common components/bq25756

if [[ -n "${CXX:-}" ]]; then
  cxx="$CXX"
elif command -v g++ >/dev/null 2>&1; then
  cxx="g++"
elif command -v clang++ >/dev/null 2>&1; then
  cxx="clang++"
else
  echo "a C++17 compiler is required (set CXX, or install g++/clang++)" >&2
  exit 1
fi

build_dir="$(mktemp -d "${TMPDIR:-/tmp}/esphome-components-host-tests.XXXXXX")"
trap 'rm -rf "$build_dir"' EXIT

common_flags=(
  -std=c++17
  -Wall
  -Wextra
  -Werror
  -Wpedantic
  -fno-exceptions
  -fno-rtti
  -I.
)

"$cxx" "${common_flags[@]}" \
  tests/component_common_test.cpp \
  -o "$build_dir/component_common_test"
"$build_dir/component_common_test"

"$cxx" "${common_flags[@]}" \
  tests/bq25756_service_test.cpp \
  components/bq25756/bq25756_protocol.cpp \
  components/bq25756/bq25756_service.cpp \
  -o "$build_dir/bq25756_service_test"
"$build_dir/bq25756_service_test"

echo "host tests: passed ($cxx)"
