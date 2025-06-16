#!/bin/bash
# detect_environment.sh - auto-detect MIDASSYS and write .env file

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/.helpers.sh"

DEBUG=false

show_help() {
  cat << EOF
Usage: $0 [OPTIONS]

Options:
  -d, --debug       Enable debug output
  -h, --help        Show this help message and exit
EOF
}

# Parse CLI args
while [[ $# -gt 0 ]]; do
  case $1 in
    -d|--debug) DEBUG=true; shift ;;
    -h|--help) show_help; exit 0 ;;
    *) echo "Unknown option: $1" >&2; show_help; exit 1 ;;
  esac
done

# Define multiple search roots
SEARCH_ROOTS=("$HOME" "/opt" "/usr/local" "/usr")
REQUIRED_FILES=("MidasConfig.cmake" "include/midas.h")

ENV_FILE="$SCRIPT_DIR/.env"

echo "[INFO] Searching for MIDAS installation, this may take some time ..."

$DEBUG && echo "[DEBUG] Searching for MIDAS installation in: ${SEARCH_ROOTS[*]}" >&2

if found=$(find_root_with_files "${SEARCH_ROOTS[@]}" -- "${REQUIRED_FILES[@]}" "$DEBUG"); then
  echo "Found MIDASSYS: $found"
else
  echo "ERROR: MIDAS installation not found." >&2
  exit 1
fi

cat > "$ENV_FILE" <<EOF
export MIDASSYS=$found
EOF

echo "[INFO] .env file written at: $ENV_FILE"
echo "[INFO] Run 'source $ENV_FILE' before building."
