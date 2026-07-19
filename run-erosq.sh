#!/bin/sh
# Launch OpenJazz on the EROS Q: free the DRM master (kill UI), present via
# drm_out, read DAP buttons via evdev, ALSA audio. Restore UI on exit.
DIR=/media/sd/ojspike
cd "$DIR" || exit 2
export HOME="$DIR" XDG_CONFIG_HOME="$DIR/.config" XDG_DATA_HOME="$DIR/.local/share"
mkdir -p "$XDG_CONFIG_HOME" "$XDG_DATA_HOME"

for pid in $(ps w 2>/dev/null | grep 'erosq-ui ||' | grep -v grep | awk '{print $1}'); do
  kill -9 "$pid" 2>/dev/null
done
killall -9 erosq-ui erosq-splash 2>/dev/null
i=0
while [ "$(pidof erosq-ui erosq-splash 2>/dev/null | wc -w)" != 0 ] && [ $i -lt 10 ]; do
  killall -9 erosq-ui erosq-splash 2>/dev/null; sleep 1; i=$((i+1))
done
sleep 1

SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=alsa LD_LIBRARY_PATH=/usr/lib ./OpenJazz "$DIR/jazz"

/etc/init.d/S90erosq-ui start
