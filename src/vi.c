#include <windows.h>
#include <stdbool.h>
#include <ctype.h>
#include "keymap.h"
#include "output.h"

// ─── Mode Switching ──────────────────────────────────────────────────────────

void vi_enter_normal(void) {
    g_state.mode        = MODE_NORMAL;
    g_state.operator_vk = 0;
    g_state.digit_len   = 0;
    layer_pop();          // remove insert layer
    layer_push(1);        // push normal layer
}

void vi_enter_insert(void) {
    g_state.mode        = MODE_INSERT;
    g_state.operator_vk = 0;
    g_state.digit_len   = 0;
    layer_pop();          // remove normal layer
    layer_push(0);        // push insert layer
}

// ─── Digit Buffer ────────────────────────────────────────────────────────────

// returns the current count prefix, defaulting to 1 if none entered
static int vi_get_count(void) {
    if (g_state.digit_len == 0) return 1;
    g_state.digit_buf[g_state.digit_len] = '\0';
    return atoi(g_state.digit_buf);
}

static void vi_clear_count(void) {
    g_state.digit_len = 0;
}

// ─── Key Processing ──────────────────────────────────────────────────────────

// returns true if the key was consumed, false if it should be passed through
bool vi_process_key(WORD vk, bool keydown) {
    if (!keydown) return false; // only act on key-down events

    // ── caps lock = mode toggle ───────────────────────────
    if (vk == VK_CAPITAL) {
        if (g_state.mode == MODE_INSERT) {
            vi_enter_normal();
        } else {
            vi_enter_insert();
        }
        return true; // always consume caps
    }

    // ── insert mode: pass everything through ─────────────
    if (g_state.mode == MODE_INSERT) {
        return false;
    }

    // ── normal mode ───────────────────────────────────────

    // accumulate digit prefix (1-9 to start, 0-9 thereafter)
    // note: '0' on its own maps to Home, so only treat it as
    // a digit if we already have digits buffered
    bool is_digit = (vk >= '1' && vk <= '9') ||
                    (vk == '0' && g_state.digit_len > 0);
    if (is_digit) {
        if (g_state.digit_len < (int)(sizeof(g_state.digit_buf) - 1)) {
            g_state.digit_buf[g_state.digit_len++] = (char)('0' + (vk - '0'));
        }
        return true;
    }

    // ── operator-pending mode ─────────────────────────────
    if (g_state.mode == MODE_OPERATOR_PENDING) {
        WORD op = g_state.operator_vk;
        g_state.mode        = MODE_NORMAL;
        g_state.operator_vk = 0;

        if (op == 'D' && vk == 'D') {
            // dd — delete line
            static key_event_t seq[] = {
                { VK_HOME,    0, 0                              },
                { VK_HOME,    0, KEYEVENTF_KEYUP                },
                { VK_SHIFT,   0, 0                              },
                { VK_END,     0, 0                              },
                { VK_END,     0, KEYEVENTF_KEYUP                },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP                },
                { VK_DELETE,  0, 0                              },
                { VK_DELETE,  0, KEYEVENTF_KEYUP                },
            };
            int count = vi_get_count();
            for (int i = 0; i < count; i++) {
                output_sequence(seq, 8);
            }
            vi_clear_count();
            return true;
        }

        if (op == 'Y' && vk == 'Y') {
            // yy — yank line
            static key_event_t seq[] = {
                { VK_HOME,    0, 0               },
                { VK_HOME,    0, KEYEVENTF_KEYUP },
                { VK_SHIFT,   0, 0               },
                { VK_END,     0, 0               },
                { VK_END,     0, KEYEVENTF_KEYUP },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP },
                { VK_CONTROL, 0, 0               },
                { 'C',        0, 0               },
                { 'C',        0, KEYEVENTF_KEYUP },
                { VK_CONTROL, 0, KEYEVENTF_KEYUP },
            };
            output_sequence(seq, 10);
            vi_clear_count();
            return true;
        }

        // unrecognised two-key sequence — discard and return to normal
        vi_clear_count();
        return true;
    }

    // ── single-key normal mode actions ───────────────────
    int count = vi_get_count();
    vi_clear_count();

    // keys that enter operator-pending mode
    if (vk == 'D' || vk == 'Y' || vk == 'C') {
        g_state.mode        = MODE_OPERATOR_PENDING;
        g_state.operator_vk = vk;
        return true;
    }

    // look up the keymap for everything else
    key_action_t *action = keymap_lookup(vk);
    if (!action) return false; // passthrough

    for (int i = 0; i < count; i++) {
        switch (action->type) {
            case ACTION_KEY:
                output_tap(action->vk);
                break;
            case ACTION_SEQUENCE:
                output_sequence(action->sequence.events, action->sequence.count);
                break;
            case ACTION_FUNCTION:
                if (action->fn) action->fn();
                break;
            case ACTION_MODE_SWITCH:
                if (action->mode == MODE_INSERT) vi_enter_insert();
                else vi_enter_normal();
                break;
            default:
                break;
        }
    }

    return true;
}