#!/usr/bin/env bash

PID1="$1"
PID2="$2"

if [ -z "$PID1" ] || [ -z "$PID2" ]; then
    read -p "Enter First PID: " PID1
    read -p "Enter Second PID: " PID2
fi

if [ -z "$PID1" ] || [ -z "$PID2" ]; then
    echo "Error: Two PIDs are required."
    exit 1
fi

INFO1=$(ps aux | grep -E "^\S+\s+$PID1\s" | grep -v "grep")
INFO2=$(ps aux | grep -E "^\S+\s+$PID2\s" | grep -v "grep")

if [ -z "$INFO1" ] || [ -z "$INFO2" ]; then
    echo "Error: One or both PIDs ($PID1, $PID2) are not running."
    exit 1
fi

ETIMES1=$(ps -p "$PID1" -o etimes= | tr -d ' ')
ETIMES2=$(ps -p "$PID2" -o etimes= | tr -d ' ')

START1=$(ps -p "$PID1" -o lstart=)
START2=$(ps -p "$PID2" -o lstart=)

echo "========================================================"
echo "PID $PID1 Start Time : $START1 ($ETIMES1 seconds ago)"
echo "PID $PID2 Start Time : $START2 ($ETIMES2 seconds ago)"
echo "========================================================"

if [ "$ETIMES1" -gt "$ETIMES2" ]; then
    echo "Oldest PID: $PID1"
elif [ "$ETIMES2" -gt "$ETIMES1" ]; then
    echo "Oldest PID: $PID2"
else
    TICKS1=$(tr '\0' '\n' < "/proc/$PID1/cmdline" 2>/dev/null | sed -n 's/.*--launch-time-ticks=\([0-9]*\).*/\1/p')
    TICKS2=$(tr '\0' '\n' < "/proc/$PID2/cmdline" 2>/dev/null | sed -n 's/.*--launch-time-ticks=\([0-9]*\).*/\1/p')

    echo "Processes started within the same second. Comparing CEF launch ticks..."
    echo "PID $PID1 Ticks: ${TICKS1:-N/A}"
    echo "PID $PID2 Ticks: ${TICKS2:-N/A}"

    if [ -n "$TICKS1" ] && [ -n "$TICKS2" ]; then
        if [ "$TICKS1" -lt "$TICKS2" ]; then
            echo "Oldest PID: $PID1"
        else
            echo "Oldest PID: $PID2"
        fi
    else
        echo "Oldest PID (default): $PID1"
    fi
fi

read -p "Do you want to send SIGUSR1 to unpause $PID1 and $PID2? (y/N): " RESUME_CHOICE

case "$RESUME_CHOICE" in
    [yY][eE][sS]|[yY])
        echo "Sending SIGUSR1 to $PID1 and $PID2..."
        kill -SIGUSR1 "$PID1" "$PID2"
        echo "Signal sent successfully."
        ;;
    *)
        echo "Skipped signaling processes."
        ;;
esac
