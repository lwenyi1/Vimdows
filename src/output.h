#ifndef OUTPUT_H
#define OUTPUT_H

#include <windows.h>
#include "keymap.h"

void output_init(void);
void output_key(WORD vk, bool keydown);
void output_tap(WORD vk);
void output_sequence(const key_event_t *events, int count);
void output_sequence_repeat(const key_event_t *events, int count);
void output_release_all(void);   // call before unhooking to clear stuck keys

#endif // OUTPUT_H