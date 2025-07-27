#!/bin/bash

# A simple integration test for logkeys.
#
# This test creates a fake input device using libevdev,
# then runs logkeys to capture a few simulated keystrokes from this device.
# Finally, it checks that the keystrokes were logged correctly.

set -euo pipefail

INPUT_EVENTS_TXT="$(dirname "$0")/input_events.txt"
LOGKEYS_BIN="$(dirname "$0")/../build/src/logkeys"
FAKE_INPUT_DEV="/tmp/fake-input-dev"
LOG_FILE="/tmp/logkeys.log"

if [ ! -x "$LOGKEYS_BIN" ]; then
    echo "Error: logkeys binary not found at $LOGKEYS_BIN"
    exit 1
fi

# Clean up from previous runs
rm -f "$FAKE_INPUT_DEV" "$LOG_FILE"

# Create a fake input device and feed it some events.
# This needs to run in the background since libevdev-events-text
# will block waiting for a client to connect.
python3 -c '
import libevdev
import sys
import time

d = libevdev.Device()
d.name = "fake-input-device"
for line in sys.stdin:
    line = line.strip()
    if not line or line.startswith("#"):
        continue
    event_type, event_code, value = line.split()
    event_type = getattr(libevdev.EV, event_type)
    event_code = getattr(libevdev.EV, event_code)
    value = int(value)
    d.create_event(event_type, event_code, value)
    d.create_event(libevdev.EV.EV_SYN, libevdev.EV.SYN_REPORT, 0)
with open("/tmp/fake-input-dev", "wb") as f:
    f.write(d.fd.read())
' < "$INPUT_EVENTS_TXT" &
evdev_pid=$!

# Wait for the fake input device to be created
sleep 0.1

# Run logkeys in the background
"$LOGKEYS_BIN" --start --device "$FAKE_INPUT_DEV" --output "$LOG_FILE"
logkeys_pid=$(pgrep logkeys)

# Wait for logkeys to start and log some keys
sleep 0.5

# Stop logkeys
kill "$logkeys_pid"

# Stop the event generator
kill "$evdev_pid"

# Wait for processes to exit
wait "$logkeys_pid" || true
wait "$evdev_pid" || true

# Check that the log file contains the expected output
# The expected output is "hello<enter>"
EXPECTED_OUTPUT="hello<enter>"
ACTUAL_OUTPUT=$(cat "$LOG_FILE" | grep -v '^#' | tr -d '\n')
if [ "$ACTUAL_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test passed!"
    exit 0
else
    echo "Test failed!"
    echo "Expected output: $EXPECTED_OUTPUT"
    echo "Actual output:   $ACTUAL_OUTPUT"
    exit 1
fi
