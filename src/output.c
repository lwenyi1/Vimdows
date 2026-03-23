/**
 * @file output.c
 * @brief Output helper functions
 * 
 * Handles output of key events to the system.
 */

#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include "output.h"
#include "keymap.h"

// track which vk codes we have currently held down so we can release
// them all cleanly on exit
static bool g_held[256] = {0};

void output_init(void) {
    memset(g_held, 0, sizeof(g_held));
}

// emit a single key press or release via SendInput
void output_key(WORD vk, bool keydown) {
    INPUT input      = {0};
    input.type       = INPUT_KEYBOARD;
    input.ki.wVk     = vk;
    input.ki.dwFlags = keydown ? 0 : KEYEVENTF_KEYUP;

    // derive scan code from vk for better compatibility
    input.ki.wScan   = (WORD)MapVirtualKey(vk, MAPVK_VK_TO_VSC);

    if (vk < 256) {
        g_held[vk] = keydown;
    }

    SendInput(1, &input, sizeof(INPUT));
}

// press then release a single key
void output_tap(WORD vk) {
    output_key(vk, true);
    output_key(vk, false);
}

// send a sequence of key events atomically in a single SendInput call
// this prevents other events from interleaving between steps
void output_sequence(const key_event_t *events, int count) {
    if (!events || count <= 0) return;

    INPUT *inputs = (INPUT *)calloc(count, sizeof(INPUT));
    if (!inputs) return;

    for (int i = 0; i < count; i++) {
        inputs[i].type       = INPUT_KEYBOARD;
        inputs[i].ki.wVk     = events[i].vk;
        inputs[i].ki.wScan   = events[i].scan
                                    ? events[i].scan
                                    : (WORD)MapVirtualKey(events[i].vk, MAPVK_VK_TO_VSC);
        inputs[i].ki.dwFlags = events[i].flags;
    }

    SendInput((UINT)count, inputs, sizeof(INPUT));
    free(inputs);
}

void output_sequence_repeat(const key_event_t *events, int count, int repeat) {
    output_sequence(events, count);
    Sleep(5);
}

// release every key we currently think is held down
// call this before unhooking to prevent stuck modifiers
void output_release_all(void) {
    for (int vk = 0; vk < 256; vk++) {
        if (g_held[vk]) {
            output_key((WORD)vk, false);
        }
    }
}