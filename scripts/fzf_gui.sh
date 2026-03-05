#!/usr/bin/env bash
set -euo pipefail

if ! command -v fzf >/dev/null 2>&1; then
    echo "Error: fzf is not installed." >&2
    echo "Install it first, for example: sudo apt install fzf" >&2
    exit 1
fi

if ! command -v quicksig >/dev/null 2>&1; then
    if [[ -x "./quicksig" ]]; then
        QUICKSIG_CMD="./quicksig"
    else
        echo "Error: quicksig not found in PATH. Build/install first." >&2
        exit 1
    fi
else
    QUICKSIG_CMD="quicksig"
fi

selection="$(
    ps -eo pid=,comm=,args= --sort=comm | \
        awk '{pid=$1; $1=""; sub(/^ /, ""); printf "%-8s %s\n", pid, $0}' | \
        fzf --multi --prompt "Select processes to kill > " --header "TAB to mark, ENTER to confirm"
    )" || true

if [[ -z "$selection" ]]; then
    echo "No processes selected."
    exit 0
fi

mapfile -t pids < <(awk '{print $1}' <<<"$selection")

cmd=("$QUICKSIG_CMD" --yes --force)
for pid in "${pids[@]}"; do
    cmd+=(--pid "$pid")
done

"${cmd[@]}"
