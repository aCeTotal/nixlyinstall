#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INTERVAL="${INTERVAL:-3}"

show_passive() {
  local msg="$1"
  if command -v kdialog >/dev/null 2>&1; then
    # short passive popup; non-blocking
    kdialog --passivepopup "$msg" 2 >/dev/null || true
  elif command -v notify-send >/dev/null 2>&1; then
    notify-send -u low "Internettilgang" "$msg" >/dev/null 2>&1 || true
  fi
}

attempt=1
while true; do
  if python3 "$SCRIPT_DIR/internet_check.py" --quiet; then
    show_passive "✔ Du har tilgang til internett."
    exit 0
  else
    show_passive "Ingen internettilgang. Sjekker igjen… (#$attempt)"
  fi
  attempt=$((attempt+1))
  sleep "$INTERVAL"
done
