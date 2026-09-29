#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT
cd "$repo_dir"

./snake > "$tmp_dir/default.snk"
[[ -s "$tmp_dir/default.snk" ]]

./snake --stdin < tests/01-simple-in.snk > "$tmp_dir/stdin.snk"
diff -q tests/01-simple-ref.snk "$tmp_dir/stdin.snk"

if ./snake -i "$tmp_dir/missing.snk" > "$tmp_dir/missing.stdout" 2> "$tmp_dir/missing.stderr"; then
  printf 'The simulator accepted a missing input file.\n' >&2
  exit 1
else
  exit_code=$?
  if (( exit_code != 255 )); then
    printf 'Missing input returned %d instead of 255.\n' "$exit_code" >&2
    exit 1
  fi
fi

printf 'All command-line checks passed.\n'
