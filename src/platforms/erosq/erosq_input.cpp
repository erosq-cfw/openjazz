#include "erosq_input.h"
#include "SDL.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>   /* struct input_event, EV_KEY, EV_REL, REL_X */

#define MAXFDS 10
static int fds[MAXFDS];
static int nfds;

/* Scroll wheel emits EV_REL/REL_X ticks with no release; hold the synthetic key
 * for one frame (press this poll, release next) so edge-detected controls
 * (release(C_CHANGE)) register it. */
static SDL_Keycode wheel_pending = SDLK_UNKNOWN;

/* EROS Q physical buttons -> the SDL keysyms erosq.h binds the controls to.
 * Play = jump/confirm (LCTRL), Menu = fire (LALT), Back = cancel (ESC),
 * Prev/Next/Vol = d-pad, wheel = change weapon (LSHIFT). Power is left to OS. */
static SDL_Keycode map_key(int evcode)
{
	switch (evcode) {
	case 165: return SDLK_LEFT;    /* KEY_PREVIOUSSONG */
	case 163: return SDLK_RIGHT;   /* KEY_NEXTSONG     */
	case 115: return SDLK_UP;      /* KEY_VOLUMEUP     */
	case 114: return SDLK_DOWN;    /* KEY_VOLUMEDOWN   */
	case 164: return SDLK_LCTRL;   /* KEY_PLAYPAUSE -> jump / confirm */
	case 139: return SDLK_LALT;    /* KEY_MENU      -> fire           */
	case 158: return SDLK_ESCAPE;  /* KEY_BACK      -> menu / cancel  */
	/* BT gamepad: movement/fire/jump come from the SDL joystick path; only the
	 * menu needs a deterministic evdev binding (SDL button indices unreliable). */
	case 315: return SDLK_ESCAPE;  /* START  -> in-game menu */
	case 314: return SDLK_p;       /* SELECT -> pause (DEFAULT_KEY_PAUSE == SDLK_p) */
	default:  return SDLK_UNKNOWN;
	}
}

static void push_key(SDL_Keycode sym, bool down)
{
	if (sym == SDLK_UNKNOWN)
		return;
	SDL_Event e;
	SDL_zero(e);
	e.type = down ? SDL_KEYDOWN : SDL_KEYUP;
	e.key.state = down ? SDL_PRESSED : SDL_RELEASED;
	e.key.keysym.sym = sym;
	e.key.keysym.scancode = SDL_GetScancodeFromKey(sym);
	SDL_PushEvent(&e);
}

void erosq_input_init(void)
{
	char path[32];
	for (int i = 0; i < MAXFDS; ++i) {
		snprintf(path, sizeof path, "/dev/input/event%d", i);
		int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (fd >= 0)
			fds[nfds++] = fd;
	}
	if (getenv("EROSQ_INDBG"))
		fprintf(stderr, "erosq_input: opened %d event fds\n", nfds);
}

void erosq_input_poll(void)
{
	if (wheel_pending != SDLK_UNKNOWN) {
		push_key(wheel_pending, false);
		wheel_pending = SDLK_UNKNOWN;
	}

	struct input_event ev;
	for (int i = 0; i < nfds; ++i) {
		while (read(fds[i], &ev, sizeof ev) == (ssize_t)sizeof ev) {
			if (getenv("EROSQ_INDBG") && ev.type != EV_SYN)
				fprintf(stderr, "erosq_input: fd%d type=%d code=%d val=%d\n",
				        i, ev.type, ev.code, ev.value);
			if (ev.type == EV_KEY) {
				if (ev.value == 2)   /* autorepeat */
					continue;
				push_key(map_key(ev.code), ev.value != 0);
			} else if (ev.type == EV_REL && ev.code == REL_X) {
				push_key(SDLK_LSHIFT, true);
				wheel_pending = SDLK_LSHIFT;
			}
		}
	}
}

void erosq_input_deinit(void)
{
	for (int i = 0; i < nfds; ++i)
		close(fds[i]);
	nfds = 0;
}
