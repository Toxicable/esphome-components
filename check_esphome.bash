#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$repo_root"

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
        cat "$log_file"
        rm -f "$log_file"
        echo "$config: invalid schema unexpectedly passed" >&2
        exit 1
      fi
      while IFS= read -r expected; do
        [[ -z "$expected" ]] && continue
        if ! grep -F -- "$expected" "$log_file" >/dev/null; then
          cat "$log_file"
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
      esphome compile "$config"
      ;;
  esac
done
