#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

SHELL_NAME=$(basename "$SHELL")
if [ "$SHELL_NAME" = "zsh" ]; then
    RC_FILE="$HOME/.zshrc"
elif [ "$SHELL_NAME" = "bash" ]; then
    RC_FILE="$HOME/.bashrc"
else
    RC_FILE="$HOME/.profile"
fi

echo "Uninstalling utils from: $SCRIPT_DIR"
echo "Target shell config:     $RC_FILE"
echo "--------------------------------------------------------"

if [ ! -f "$RC_FILE" ]; then
    echo "Error: Shell configuration file '$RC_FILE' not found."
    exit 1
fi

cp "$RC_FILE" "${RC_FILE}.bak"

MARKER="# --- PILOT UTIL SCRIPT ---"
COUNT=0

for file in "$SCRIPT_DIR"/*; do
    [ -f "$file" ] || continue

    filename=$(basename "$file")

    if [[ "$filename" == "install_utils.sh" ]] || [[ "$filename" == "uninstall_utils.sh" ]]; then
        continue
    fi

    alias_name="${filename%.*}"
    full_path="$file"
    alias_line="alias $alias_name=\"$full_path\""

    if grep -Fq "$alias_line" "$RC_FILE"; then
        grep -Fv "$alias_line" "$RC_FILE" > "${RC_FILE}.tmp" && mv "${RC_FILE}.tmp" "$RC_FILE"
        echo "Removed alias:   $alias_name -> $full_path"
        ((COUNT++))
    else
        echo "Not present:     $alias_name"
    fi
done

if ! grep -q "alias .*=\"$SCRIPT_DIR/" "$RC_FILE"; then
    grep -Fv "$MARKER" "$RC_FILE" > "${RC_FILE}.tmp" && mv "${RC_FILE}.tmp" "$RC_FILE"
    sed -i -e :a -e '/^\n*$/{$d;N;ba' -e '}' "$RC_FILE" 2>/dev/null || true
fi

echo "--------------------------------------------------------"
if [ "$COUNT" -gt 0 ]; then
    echo "Successfully removed $COUNT alias(es) from $RC_FILE."
    echo "Backup saved to: ${RC_FILE}.bak"
    echo "Run 'source $RC_FILE' or open a new terminal tab to apply changes."
else
    echo "No aliases were removed."
    rm -f "${RC_FILE}.bak"
fi
