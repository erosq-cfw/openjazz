#include "erosq_input.h"
#include "SDL.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/input.h>   /* struct input_event, EV_KEY, EV_REL, REL_X */

/* Slot i is /dev/input/eventi, or -1 if not open. Indexing by node number (not
 * packing into an nfds count) is what lets a device come and go: a BT pad's node
 * vanishes on disconnect and the kernel does NOT close our fd, it just fails
 * with ENODEV forever, while the reconnect creates a new node at the same path.
 * Without the reopen in the poll loop, unplugging the pad killed its evdev keys
 * (START/SELECT below) until OpenJazz was restarted. */
#define MAXFDS 10
static int fds[MAXFDS];

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

static void open_missing(void)
{
	char path[32];
	for (int i = 0; i < MAXFDS; ++i) {
		if (fds[i] >= 0)
			continue;
		snprintf(path, sizeof path, "/dev/input/event%d", i);
		fds[i] = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (fds[i] >= 0 && getenv("EROSQ_INDBG"))
			fprintf(stderr, "erosq_input: opened event%d\n", i);
	}
}

void erosq_input_init(void)
{
	for (int i = 0; i < MAXFDS; ++i)
		fds[i] = -1;
	open_missing();
}

void erosq_input_poll(void)
{
	if (wheel_pending != SDLK_UNKNOWN) {
		push_key(wheel_pending, false);
		wheel_pending = SDLK_UNKNOWN;
	}

	struct input_event ev;
	static unsigned rescan;
	for (int i = 0; i < MAXFDS; ++i) {
		ssize_t n;
		if (fds[i] < 0)
			continue;
		while ((n = read(fds[i], &ev, sizeof ev)) == (ssize_t)sizeof ev) {
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
		if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			close(fds[i]);       /* device went away (ENODEV) */
			fds[i] = -1;
		}
	}
	/* ~1s at any sane frame rate; probing 10 paths every frame would be silly */
	if ((++rescan & 63) == 0)
		open_missing();
}

void erosq_input_deinit(void)
{
	for (int i = 0; i < MAXFDS; ++i) {
		if (fds[i] >= 0)
			close(fds[i]);
		fds[i] = -1;
	}
}
