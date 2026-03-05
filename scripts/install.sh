#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_DIR"
make

install -m 0755 quicksig /usr/local/bin/quicksig
install -m 0755 scripts/fzf_gui.sh /usr/local/bin/quicksig-gui

echo "Installed: /usr/local/bin/quicksig"
echo "Installed: /usr/local/bin/quicksig-gui"
echo "Try: quicksig --help"
