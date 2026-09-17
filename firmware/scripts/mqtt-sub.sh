#!/usr/bin/env bash
set -euo pipefail

# Subscribes to a Bambu printer's local MQTT report topic using mqttx,
# for watching the messages OpenSpool sends/receives over LAN-only MQTT.
#
# Fill in the three values below, then run:
#   ./mqtt-sub.sh
#
# Find these on the printer's touchscreen: Settings -> Network
# (or copy them from the OpenSpool web dashboard's "Printer Serial Number",
# "Printer IP Address", and "Printer Lan Access Code" fields, which are
# known-good since the device is already connecting successfully with them).

SERIAL_NUMBER="0300CA652204315"       # e.g. 01P00A000000000
LAN_ACCESS_CODE="5484e9ac"     # e.g. 12345678
BAMBU_IP_ADDRESS="192.168.0.178"    # e.g. 192.168.1.50

if [[ -z "$SERIAL_NUMBER" || -z "$LAN_ACCESS_CODE" || -z "$BAMBU_IP_ADDRESS" ]]; then
  echo "Error: fill in SERIAL_NUMBER, LAN_ACCESS_CODE, and BAMBU_IP_ADDRESS at the top of this script first." >&2
  exit 1
fi

if ! command -v mqttx >/dev/null 2>&1; then
  echo "Error: mqttx CLI not found. Install it with: npm install -g mqttx-cli" >&2
  exit 1
fi

TOPIC="device/${SERIAL_NUMBER}/report"
echo "Subscribing to ${TOPIC} on ${BAMBU_IP_ADDRESS}:8883 ..."

mqttx sub \
  -t "$TOPIC" \
  -u bblp \
  -P "$LAN_ACCESS_CODE" \
  --mqtt-version 3.1.1 \
  -h "$BAMBU_IP_ADDRESS" \
  -p 8883 \
  -l mqtts \
  --insecure
