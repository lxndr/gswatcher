#!/usr/bin/env bash
# Verify AppImage embeds an executable gswatcher (AppRun.wrapped is a symlink to it).
set -euo pipefail

APPIMAGE="${1:?Usage: $0 path/to/AppImage}"
LOG_PATH="${DEBUG_LOG_PATH:-$(dirname "$0")/../.cursor/debug-4bcf49.log}"
SESSION_ID="${DEBUG_SESSION_ID:-4bcf49}"

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

cd "$tmpdir"
"$APPIMAGE" --appimage-extract >/dev/null 2>&1

gs_mode="$(stat -c '%a' squashfs-root/usr/bin/gswatcher)"
ar_mode="$(stat -c '%a' squashfs-root/AppRun)"
wrap_mode="$(stat -c '%a' squashfs-root/AppRun.wrapped)"
wrap_type="$(stat -c '%F' squashfs-root/AppRun.wrapped)"

mkdir -p "$(dirname "$LOG_PATH")"
printf '%s\n' "{\"sessionId\":\"$SESSION_ID\",\"runId\":\"local\",\"hypothesisId\":\"A\",\"location\":\"verify-appimage-perms.sh\",\"message\":\"extracted squashfs modes\",\"data\":{\"gswatcher\":\"$gs_mode\",\"AppRun\":\"$ar_mode\",\"AppRun.wrapped\":\"$wrap_mode\",\"wrapped_type\":\"$wrap_type\"},\"timestamp\":$(date +%s%3N)}" >> "$LOG_PATH"

echo "usr/bin/gswatcher: $gs_mode ($wrap_type AppRun.wrapped -> usr/bin/gswatcher)"
if [[ "$gs_mode" != *5* && "$gs_mode" != *7* ]]; then
  echo "ERROR: gswatcher is not executable inside AppImage; AppRun will fail with Permission denied." >&2
  exit 1
fi
echo "OK: gswatcher is executable inside AppImage"
