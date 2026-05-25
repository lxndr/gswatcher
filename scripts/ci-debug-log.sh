#!/usr/bin/env bash
# #region agent log
# Appends one NDJSON debug line for CI/local MSYS2 diagnosis.
ci_debug_log() {
  local hypothesis_id="$1" message="$2" data="${3:-{}}"
  local log_file="${DEBUG_LOG_PATH:-${GITHUB_WORKSPACE:-.}/.cursor/debug-cdb5c2.log}"
  mkdir -p "$(dirname "$log_file")"
  local ts
  ts=$(date +%s)000
  printf '%s\n' "{\"sessionId\":\"cdb5c2\",\"hypothesisId\":\"${hypothesis_id}\",\"location\":\"build-windows\",\"message\":\"${message}\",\"data\":${data},\"timestamp\":${ts},\"runId\":\"${GITHUB_RUN_ID:-local}\"}" >> "$log_file"
}
# #endregion
