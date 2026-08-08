#!/bin/bash
# Starts the vision process and the game together, and — importantly —
# makes sure the vision process dies when the game does.
#
# Why the trap matters: a leftover Python process keeps holding the
# webcam open, so the NEXT run would fail to open the camera at all.

cd "$(dirname "$0")"

echo "[run] Starting vision process..."
python3 vision/server.py &
VISION_PID=$!

# Kill the vision process however this script exits -- normal exit,
# Ctrl+C, or error.
trap 'kill $VISION_PID 2>/dev/null' EXIT

# Cosmetic only: VisionClient retries the connection on its own, but this
# gives the camera a moment to warm up so you don't see retry messages.
sleep 2

echo "[run] Starting game..."
./build/DeadAim