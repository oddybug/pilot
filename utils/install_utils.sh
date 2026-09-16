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

echo "Installing utils from: $SCRIPT_DIR"
echo "Target shell config:   $RC_FILE"
echo "--------------------------------------------------------"

MARKER="# --- PILOT UTIL SCRIPT ---"
if ! grep -q "$MARKER" "$RC_FILE"; then
    echo -e "\n$MARKER" >> "$RC_FILE"
fi

COUNT=0

for file in "$SCRIPT_DIR"/*; do
    [ -f "$file" ] || continue

    filename=$(basename "$file")

    if [[ "$filename" == "install_utils.sh" ]] || [[ "$filename" == "uninstall_utils.sh" ]]; then
        continue
    fi

    chmod +x "$file"

    alias_name="${filename%.*}"
    full_path="$file"
    alias_line="alias $alias_name=\"$full_path\""

    if grep -Fq "$alias_line" "$RC_FILE"; then
        echo "Already installed: $alias_name -> $full_path"
    else
        echo "$alias_line" >> "$RC_FILE"
        echo "Installed alias:   $alias_name -> $full_path"
        ((COUNT++))
    fi
done

echo "--------------------------------------------------------"
if [ "$COUNT" -gt 0 ]; then
    echo "Successfully added $COUNT alias(es) to $RC_FILE."
    echo "Run 'source $RC_FILE' or open a new terminal tab to activate them."
else
    echo "No new aliases added."
fi
