#ifndef EROSQ_INPUT_H
#define EROSQ_INPUT_H

/* evdev button reader for the EROS Q. Opens /dev/input/event*, and on each poll
 * drains events and injects synthetic SDL keydown/keyup so OpenJazz's Controls
 * consumes them through its normal SDL_PollEvent path (no engine changes). */
void erosq_input_init(void);
void erosq_input_poll(void);
void erosq_input_deinit(void);

#endif
