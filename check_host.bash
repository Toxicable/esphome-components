#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$repo_root"

python3 tools/check_core_purity.py \
  components/component_common \
  components/bq25756 \
  components/bq76952/bq76952_registers.h \
  components/bq76952/bq76952_status.h \
  components/bq76952/bq76952_status.cpp \
  components/mcf83xx_common \
  components/mcf8316d/mcf8316d_bus.h \
  components/mcf8316d/mcf8316d_protocol.h \
  components/mcf8316d/mcf8316d_protocol.cpp \
  components/mcf8316d/mcf8316d_service.h \
  components/mcf8316d/mcf8316d_service.cpp \
  components/mcf8329a/mcf8329a_bus.h \
  components/mcf8329a/mcf8329a_protocol.h \
  components/mcf8329a/mcf8329a_protocol.cpp \
  components/mcf8329a/mcf8329a_service.h \
  components/mcf8329a/mcf8329a_service.cpp \
  components/mcf8329a/mcf8329a_tables.h \
  components/programmable_load/calibration.h \
  components/programmable_load/programmable_load_core.h \
  components/programmable_load/programmable_load_core.cpp

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

"$cxx" "${common_flags[@]}" \
  tests/bq76952_status_test.cpp \
  components/bq76952/bq76952_status.cpp \
  -o "$build_dir/bq76952_status_test"
"$build_dir/bq76952_status_test"

"$cxx" "${common_flags[@]}" \
  tests/mcf83xx_common_test.cpp \
  -o "$build_dir/mcf83xx_common_test"
"$build_dir/mcf83xx_common_test"

"$cxx" "${common_flags[@]}" \
  tests/mcf83xx_services_test.cpp \
  components/mcf8316d/mcf8316d_protocol.cpp \
  components/mcf8316d/mcf8316d_service.cpp \
  components/mcf8329a/mcf8329a_protocol.cpp \
  components/mcf8329a/mcf8329a_service.cpp \
  -o "$build_dir/mcf83xx_services_test"
"$build_dir/mcf83xx_services_test"

"$cxx" "${common_flags[@]}" \
  tests/programmable_load_core_test.cpp \
  components/programmable_load/programmable_load_core.cpp \
  -o "$build_dir/programmable_load_core_test"
"$build_dir/programmable_load_core_test"

echo "host tests: passed ($cxx)"
