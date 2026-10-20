#!/bin/sh
# Host check for an erosq-launch.sh wrapper (this repo's or opentyrian's):
#   sh erosq-launch-test.sh [path/to/erosq-launch.sh]
# Stubs pidof/killall/mpd/sleep and the game, rewrites /media/sd to a temp dir,
# and asserts: mpd up -> killed BEFORE the game, relaunched AFTER it;
# mpd down -> neither killed nor started. Exit 0 = PASS.
W=${1:-$(dirname "$0")/erosq-launch.sh}
T=$(mktemp -d) || exit 1
trap 'rm -rf "$T"' EXIT
mkdir -p "$T/bin"
for c in killall mpd sleep; do printf '#!/bin/sh\necho %s >> "$T/log"\n' "$c" > "$T/bin/$c"; done
printf '#!/bin/sh\n[ -n "$MPD_UP" ]\n' > "$T/bin/pidof"
elf=$(sed -n 's#^/media/sd/\(.*\.elf\)$#\1#p' "$W")
[ -n "$elf" ] || { echo "FAIL: no /media/sd/*.elf line in $W"; exit 1; }
mkdir -p "$T/sd/$(dirname "$elf")"
printf '#!/bin/sh\necho game >> "$T/log"\n' > "$T/sd/$elf"
chmod +x "$T"/bin/* "$T/sd/$elf"
sed "s#/media/sd/#$T/sd/#g" "$W" > "$T/w.sh"
rc=0
run() {   # run MPD_UP expected-log
	: > "$T/log"
	env T="$T" MPD_UP="$1" PATH="$T/bin:$PATH" sh "$T/w.sh"
	got=$(tr '\n' ' ' < "$T/log")
	[ "$got" = "$2" ] && echo "PASS mpd_up=${1:-0}: $got" || { echo "FAIL mpd_up=${1:-0}: got '$got' want '$2'"; rc=1; }
}
run 1 "killall sleep game mpd "
run "" "game "
exit $rc
