
/**
 *
 * @file erosq.h
 *
 * Part of the OpenJazz project — EROS Q (Ingenic X1000) handheld target.
 *
 * @par Licence:
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _EROSQ_H
#define _EROSQ_H

#ifdef TARGET_EROSQ

/* The X1000 has no GPU and SDL2 has only the "dummy" video driver, so we never
 * open a real window: the game renders into its 8-bit 320x200 canvas and
 * erosq_video blits it to /dev/dri/card0 (letterboxed into the 320x240 panel).
 * Keep the native 320x200 canvas (DEFAULT_SCREEN_WIDTH/HEIGHT defaults) — the
 * 20px letterbox happens in the drm blit, not here. */
#define FULLSCREEN_ONLY
#define NO_RESIZE

/* DAP buttons are synthesised as these keysyms by erosq_input (see keymap
 * there). Play doubles as jump+confirm (LCTRL), like the dingoo scheme. */
#define DEFAULT_KEY_JUMP   (SDLK_LCTRL)
#define DEFAULT_KEY_SWIM   (SDLK_LCTRL)
#define DEFAULT_KEY_FIRE   (SDLK_LALT)
#define DEFAULT_KEY_CHANGE (SDLK_LSHIFT)
#define DEFAULT_KEY_ENTER  (SDLK_LCTRL)
#define DEFAULT_KEY_ESCAPE (SDLK_ESCAPE)
#define DEFAULT_KEY_YES    (SDLK_LCTRL)
#define DEFAULT_KEY_NO     (SDLK_LALT)

#endif

#endif
