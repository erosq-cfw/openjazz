#!/bin/sh
# erosq-ui Extras -> Games launcher for Jazz Jackrabbit. Install THIS script as
# /media/sd/ojspike/OpenJazz (the extras.tsv path) and the binary from
# build-erosq.sh beside it as OpenJazz.elf. Never copy a binary over OpenJazz:
# a 07-25 deploy did and silently dropped the mpd kill/relaunch below.
# Kill mpd to free the DAC (no dmix) and its RAM/swap, run the game, then
# relaunch mpd -- it resumes the queue+position from its state_file (mpd.conf);
# do NOT `mpc stop` first (persists state=stop, kills the resume).
# Env/cwd are left as erosq-ui passes them (the binary is self-contained).
was_up=; pidof mpd >/dev/null 2>&1 && was_up=1
[ "$was_up" ] && { killall mpd 2>/dev/null; sleep 1; }
/media/sd/ojspike/OpenJazz.elf
[ "$was_up" ] && mpd 2>/dev/null
