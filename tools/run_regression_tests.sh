#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT

cd "$repo_dir"
count=0
for input in tests/*-in.snk; do
  name="${input%-in.snk}"
  expected="${name}-ref.snk"
  if [[ ! -f "$expected" ]]; then
    printf 'Missing expected output for %s\n' "$input" >&2
    exit 1
  fi
  output="$tmp_dir/$(basename "$name").snk"
  ./snake -i "$input" -o "$output"
  if ! diff -q "$expected" "$output"; then
    printf 'Regression failed: %s\n' "$input" >&2
    exit 1
  fi
  count=$((count + 1))
done

if (( count == 0 )); then
  printf 'No board fixtures found.\n' >&2
  exit 1
fi
printf 'All %d board regression tests passed.\n' "$count"
