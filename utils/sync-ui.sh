#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PILOT_DIR="$(dirname "$SCRIPT_DIR")"
PILOT_UI_DIR="${PILOT_UI_DIR:-$(dirname "$PILOT_DIR")/pilot-ui}"

command -v npm >/dev/null 2>&1 || {
  echo "sync-ui: npm not found in PATH" >&2
  exit 1
}

if [ ! -d "$PILOT_UI_DIR" ]; then
  echo "sync-ui: pilot-ui dir not found: $PILOT_UI_DIR" >&2
  echo "sync-ui: override with PILOT_UI_DIR=/path/to/pilot-ui" >&2
  exit 1
fi

npm --prefix "$PILOT_UI_DIR" run build

rm -rf "$PILOT_DIR/ui"
cp -r "$PILOT_UI_DIR/dist" "$PILOT_DIR/ui"
sed -i 's#href="/#href="./#g; s#src="/#src="./#g' "$PILOT_DIR/ui/index.html"

echo "sync-ui: $PILOT_UI_DIR/dist -> $PILOT_DIR/ui"
