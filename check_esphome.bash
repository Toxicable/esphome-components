#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$repo_root"

failure_log="${ESPHOME_FAILURE_LOG:-esphome-failure.log}"
rm -f "$failure_log"

record_failure() {
  local config="$1"
  local log_file="$2"
  {
    echo "fixture: $config"
    echo
    tail -n 300 "$log_file"
  } >"$failure_log"
}

if ! command -v esphome >/dev/null 2>&1; then
  echo "esphome is required; use the repository devcontainer" >&2
  exit 1
fi

if [[ $# -gt 0 ]]; then
  configs=("$@")
else
  mapfile -d '' configs < <(
    find components -mindepth 2 -maxdepth 2 -type f \
      \( -name 'test_config.yaml' -o -name 'test_invalid_*.yaml' \) \
      -print0 | sort -z
  )
fi

if [[ ${#configs[@]} -eq 0 ]]; then
  echo "no ESPHome test configurations found" >&2
  exit 1
fi

for config in "${configs[@]}"; do
  case "$(basename "$config")" in
    test_invalid_*.yaml)
      expected_file="${config%.yaml}.expected"
      if [[ ! -f "$expected_file" ]]; then
        echo "$config: missing expected-error file $expected_file" >&2
        exit 1
      fi
      log_file="$(mktemp "${TMPDIR:-/tmp}/esphome-invalid.XXXXXX")"
      if esphome config "$config" >"$log_file" 2>&1; then
        record_failure "$config" "$log_file"
        cat "$failure_log"
        rm -f "$log_file"
        echo "$config: invalid schema unexpectedly passed" >&2
        exit 1
      fi
      while IFS= read -r expected; do
        [[ -z "$expected" ]] && continue
        if ! grep -F -- "$expected" "$log_file" >/dev/null; then
          record_failure "$config" "$log_file"
          cat "$failure_log"
          rm -f "$log_file"
          echo "$config: expected error text not found: $expected" >&2
          exit 1
        fi
      done <"$expected_file"
      rm -f "$log_file"
      echo "$config: rejected as expected"
      ;;
    *)
      echo "$config: compiling"
      log_file="$(mktemp "${TMPDIR:-/tmp}/esphome-compile.XXXXXX")"
      if ! esphome compile "$config" >"$log_file" 2>&1; then
        record_failure "$config" "$log_file"
        echo "$config: compile failed" >&2
        cat "$failure_log" >&2
        rm -f "$log_file"
        exit 1
      fi
      rm -f "$log_file"
      echo "$config: compiled"
      ;;
  esac
done
