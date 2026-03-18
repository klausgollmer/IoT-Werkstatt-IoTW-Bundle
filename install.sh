#!/bin/bash

# Absoluter Pfad zu diesem Script (= IoTW-Verzeichnis)
BASE_PATH=$(cd "$(dirname "$0")" && pwd)

DESKTOP_DIR="$(xdg-user-dir DESKTOP)"

APP_NAME="MakeyLab"
EXEC_TARGET="$BASE_PATH/arduino/arduino"
ICON="$BASE_PATH/MakeyLab.png"
DESKTOP_FILE="$DESKTOP_DIR/MakeyLab.desktop"

mkdir -p "$DESKTOP_DIR"

cat <<EOF > "$DESKTOP_FILE"
[Desktop Entry]
Type=Application
Name=$APP_NAME
Exec=/bin/bash -c "$EXEC_TARGET"
Path=$BASE_PATH/arduino
Icon=$ICON
Terminal=false
EOF

chmod +x "$DESKTOP_FILE"

# Für Ubuntu, GNOME, Mint:
if command -v gio >/dev/null; then
  gio set "$DESKTOP_FILE" metadata::trusted true
fi

echo "✅ MakeyLab Desktop-Icon wurde erstellt."
