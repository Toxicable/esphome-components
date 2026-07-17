#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$repo_root"

python3 tools/check_component_inventory.py

python3 tools/check_core_purity.py \
  components/component_common \
  components/bq25756 \
  components/bq76952/bq76952_registers.h \
  components/bq76952/bq76952_status.h \
  components/bq76952/bq76952_status.cpp \
  components/mcf83xx_common \
  components/mcf8316d/mcf8316d_bus.h \
  components/mcf8316d/mcf8316d_registers.h \
  components/mcf8316d/mcf8316d_protocol.h \
  components/mcf8316d/mcf8316d_protocol.cpp \
  components/mcf8316d/mcf8316d_service.h \
  components/mcf8316d/mcf8316d_service.cpp \
  components/mcf8329a/mcf8329a_bus.h \
  components/mcf8329a/mcf8329a_registers.h \
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
  echo "a C++20 compiler is required (set CXX, or install g++/clang++)" >&2
  exit 1
fi

build_dir="$(mktemp -d "${TMPDIR:-/tmp}/esphome-components-host-tests.XXXXXX")"
trap 'rm -rf "$build_dir"' EXIT

common_flags=(
  -std=gnu++20
  -Wall
  -Wextra
  -Werror
  -Wpedantic
  -fno-exceptions
  -fno-rtti
  -I.
)

run_test() {
  local name="$1"
  shift
  echo "host test: $name"
  "$cxx" "${common_flags[@]}" "$@" -o "$build_dir/$name"
  "$build_dir/$name"
}

run_test component_common_test \
  tests/component_common_test.cpp

run_test bq25756_service_test \
  tests/bq25756_service_test.cpp \
  components/bq25756/bq25756_protocol.cpp \
  components/bq25756/bq25756_service.cpp

run_test bq76952_status_test \
  tests/bq76952_status_test.cpp \
  components/bq76952/bq76952_status.cpp

run_test mcf83xx_common_test \
  tests/mcf83xx_common_test.cpp

run_test mcf83xx_services_test \
  tests/mcf83xx_services_test.cpp \
  components/mcf8316d/mcf8316d_protocol.cpp \
  components/mcf8316d/mcf8316d_service.cpp \
  components/mcf8329a/mcf8329a_protocol.cpp \
  components/mcf8329a/mcf8329a_service.cpp

run_test programmable_load_core_test \
  tests/programmable_load_core_test.cpp \
  components/programmable_load/programmable_load_core.cpp

echo "host tests: passed ($cxx)"
