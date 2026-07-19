#!/bin/sh
# Build OpenJazz for the EROS Q (Ingenic X1000). Produces a stripped, self-
# contained mipsel binary that presents via drm_out, reads DAP buttons via
# evdev, and needs no argv/env (DATAPATH baked; SDL drivers pinned in main.cpp).
# Launched bare by erosq-ui from the extras manifest.
set -e

BR=${BR:-buildroot-2026.05}
TC="$BR/output/host/share/buildroot/toolchainfile.cmake"
STRIP="$BR/output/host/bin/mipsel-buildroot-linux-musl-strip"
DATA_DIR=${DATA_DIR:-/media/sd/ojspike/jazz}   # game data location on device

cmake -B build-port -S . \
	-DCMAKE_TOOLCHAIN_FILE="$TC" \
	-DTARGET_EROSQ=ON -DNETWORK=OFF -DSCALE=OFF \
	-DCMAKE_BUILD_TYPE=Release \
	-DDATAPATH="$DATA_DIR" \
	-DCMAKE_CXX_FLAGS="-O2 -fomit-frame-pointer" \
	-DCMAKE_C_FLAGS="-O2 -fomit-frame-pointer"
cmake --build build-port -j"$(nproc)"
"$STRIP" build-port/OpenJazz

echo "=== build-port/OpenJazz ==="
ls -la build-port/OpenJazz
"$BR/output/host/bin/mipsel-buildroot-linux-musl-readelf" -d build-port/OpenJazz | grep NEEDED
