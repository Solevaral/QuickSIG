#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_DIR"
make

install -m 0755 pkill-smart /usr/local/bin/pkill-smart
install -m 0755 scripts/fzf_gui.sh /usr/local/bin/pkill-smart-gui

echo "Installed: /usr/local/bin/pkill-smart"
echo "Installed: /usr/local/bin/pkill-smart-gui"
echo "Try: pkill-smart --help"
