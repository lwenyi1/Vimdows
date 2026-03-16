/**
 * @file vi.c
 * @brief Main vi key processing
 * 
 * Handles the main key processing logic for vi mode and the more complex
 * mappings like mode switching, digit prefix and special "operator mode"
 * cases (dd, yy). Essentially keymappings that need "awareness" of its
 * vi state.
 */


#include <windows.h>
#include <stdbool.h>
#include <ctype.h>
#include "keymap.h"
#include "output.h"

//===========================================================================
// Mode switching helper functions
//===========================================================================

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

//===========================================================================
// Digit prefix helper functions
//===========================================================================

/**
 * Returns the current count prefix, defaulting to 1 if none entered
 */
static int vi_get_count(void) {
    if (g_state.digit_len == 0) return 1;
    g_state.digit_buf[g_state.digit_len] = '\0';
    return atoi(g_state.digit_buf);
}

static void vi_clear_count(void) {
    g_state.digit_len = 0;
}

//===========================================================================
// Key processing
//===========================================================================

// returns true if the key was consumed, false if it should be passed through
bool vi_process_key(WORD vk, bool keydown) {
    if (!keydown) return false; // only act on key-down events

    //-------------------------------------------------------------------
    // Caps lock: main switch between insert and normal mode
    //-------------------------------------------------------------------
    if (vk == VK_CAPITAL) {
        if (g_state.mode == MODE_INSERT) {
            vi_enter_normal();
        } else {
            vi_enter_insert();
        }
        return true; // always consume caps
    }

    //-------------------------------------------------------------------
    // Insert mode
    //-------------------------------------------------------------------
    if (g_state.mode == MODE_INSERT) {
        return false;
    }

    //-------------------------------------------------------------------
    // Normal mode
    //-------------------------------------------------------------------

    //-------------------------------------------------------------------
    // Operator mode, e.g. d_, y_, c_
    //-------------------------------------------------------------------
    if (g_state.mode == MODE_OPERATOR_PENDING) {
        WORD op = g_state.operator_vk;
        g_state.mode        = MODE_NORMAL;
        g_state.operator_vk = 0;

        if (op == 'D' && vk == 'D') {
            // dd, copy then delete current line
            static key_event_t select_seq[] = {
                { VK_HOME,    0, 0                },
                { VK_HOME,    0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, 0                },
                { VK_END,     0, 0                },
                { VK_END,     0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP  },
            };
            static key_event_t copy_seq[] = {
                { VK_CONTROL, 0, 0                },
                { 'C',        0, 0                },
                { 'C',        0, KEYEVENTF_KEYUP  },
                { VK_CONTROL, 0, KEYEVENTF_KEYUP  },
            };
            static key_event_t delete_seq[] = {
                { VK_DELETE,  0, 0                },
                { VK_DELETE,  0, KEYEVENTF_KEYUP  },
            };

            int count = vi_get_count();
            for (int i = 0; i < count; i++) {
                output_sequence(select_seq, 6);
                output_sequence(copy_seq, 4);
                Sleep(20);  // give the clipboard time to finish writing
                output_sequence(delete_seq, 2);
            }
            vi_clear_count();
            return true;
        }
        
        if (op == 'Y' && vk == 'Y') {
            // yy, yank the current line
            static key_event_t select_seq[] = {
                { VK_HOME,    0, 0                },
                { VK_HOME,    0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, 0                },
                { VK_END,     0, 0                },
                { VK_END,     0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP  },
            };
            static key_event_t copy_seq[] = {
                { VK_CONTROL, 0, 0                },
                { 'C',        0, 0                },
                { 'C',        0, KEYEVENTF_KEYUP  },
                { VK_CONTROL, 0, KEYEVENTF_KEYUP  },
            };
            static key_event_t deselect_seq[] = {
                // Home without shift collapses the selection and
                // returns cursor to start of line, closest to vi yy behaviour
                { VK_HOME,    0, 0                },
                { VK_HOME,    0, KEYEVENTF_KEYUP  },
            };

            output_sequence(select_seq, 6);
            output_sequence(copy_seq, 4);
            Sleep(20);
            output_sequence(deselect_seq, 2);
            vi_clear_count();
            return true;
        }

        if (op == 'C' && vk == 'C') {
            // cc — delete line and enter insert mode
            static key_event_t select_seq[] = {
                { VK_HOME,    0, 0                },
                { VK_HOME,    0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, 0                },
                { VK_END,     0, 0                },
                { VK_END,     0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP  },
            };
            static key_event_t delete_seq[] = {
                { VK_DELETE,  0, 0                },
                { VK_DELETE,  0, KEYEVENTF_KEYUP  },
            };
            int count = vi_get_count();
            for (int i = 0; i < count; i++) {
                output_sequence(select_seq, 6);
                output_sequence(delete_seq, 2);
            }
            vi_clear_count();
            vi_enter_insert();
            return true;
        }

        if (op == 'D' && vk == 'W') {
            // dw — delete to next word boundary
            static key_event_t seq[] = {
                { VK_SHIFT,   0, 0                },
                { VK_CONTROL, 0, 0                },
                { VK_RIGHT,   0, 0                },
                { VK_RIGHT,   0, KEYEVENTF_KEYUP  },
                { VK_CONTROL, 0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP  },
                { VK_DELETE,  0, 0                },
                { VK_DELETE,  0, KEYEVENTF_KEYUP  },
            };
            int count = vi_get_count();
            for (int i = 0; i < count; i++) {
                output_sequence(seq, 8);
            }
            vi_clear_count();
            return true;
        }

        if (op == 'Y' && vk == 'W') {
            // yw — yank to next word boundary
            static key_event_t select_seq[] = {
                { VK_SHIFT,   0, 0                },
                { VK_CONTROL, 0, 0                },
                { VK_RIGHT,   0, 0                },
                { VK_RIGHT,   0, KEYEVENTF_KEYUP  },
                { VK_CONTROL, 0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP  },
            };
            static key_event_t copy_seq[] = {
                { VK_CONTROL, 0, 0                },
                { 'C',        0, 0                },
                { 'C',        0, KEYEVENTF_KEYUP  },
                { VK_CONTROL, 0, KEYEVENTF_KEYUP  },
            };
            static key_event_t deselect_seq[] = {
                { VK_LEFT,    0, 0                },
                { VK_LEFT,    0, KEYEVENTF_KEYUP  },
            };
            int count = vi_get_count();
            for (int i = 0; i < count; i++) {
                output_sequence(select_seq, 6);
            }
            output_sequence(copy_seq, 4);
            Sleep(20);
            output_sequence(deselect_seq, 2);
            vi_clear_count();
            return true;
        }

        if (op == 'C' && vk == 'W') {
            // cw — delete to next word boundary and enter insert mode
            static key_event_t seq[] = {
                { VK_SHIFT,   0, 0                },
                { VK_CONTROL, 0, 0                },
                { VK_RIGHT,   0, 0                },
                { VK_RIGHT,   0, KEYEVENTF_KEYUP  },
                { VK_CONTROL, 0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP  },
                { VK_DELETE,  0, 0                },
                { VK_DELETE,  0, KEYEVENTF_KEYUP  },
            };
            int count = vi_get_count();
            for (int i = 0; i < count; i++) {
                output_sequence(seq, 8);
            }
            vi_clear_count();
            vi_enter_insert();
            return true;
        }

        if (op == 'D' && vk == '4') {
            // d$ — delete to end of line
            static key_event_t seq[] = {
                { VK_SHIFT,   0, 0                },
                { VK_END,     0, 0                },
                { VK_END,     0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP  },
                { VK_DELETE,  0, 0                },
                { VK_DELETE,  0, KEYEVENTF_KEYUP  },
            };
            output_sequence(seq, 6);
            vi_clear_count();
            return true;
        }

        if (op == 'Y' && vk == '4') {
            // y$ — yank to end of line
            static key_event_t select_seq[] = {
                { VK_SHIFT,   0, 0                },
                { VK_END,     0, 0                },
                { VK_END,     0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP  },
            };
            static key_event_t copy_seq[] = {
                { VK_CONTROL, 0, 0                },
                { 'C',        0, 0                },
                { 'C',        0, KEYEVENTF_KEYUP  },
                { VK_CONTROL, 0, KEYEVENTF_KEYUP  },
            };
            static key_event_t deselect_seq[] = {
                { VK_LEFT,    0, 0                },
                { VK_LEFT,    0, KEYEVENTF_KEYUP  },
            };
            output_sequence(select_seq, 4);
            output_sequence(copy_seq, 4);
            Sleep(20);
            output_sequence(deselect_seq, 2);
            vi_clear_count();
            return true;
        }

        if (op == 'C' && vk == '4') {
            // c$ — delete to end of line and enter insert mode
            static key_event_t seq[] = {
                { VK_SHIFT,   0, 0                },
                { VK_END,     0, 0                },
                { VK_END,     0, KEYEVENTF_KEYUP  },
                { VK_SHIFT,   0, KEYEVENTF_KEYUP  },
                { VK_DELETE,  0, 0                },
                { VK_DELETE,  0, KEYEVENTF_KEYUP  },
            };
            output_sequence(seq, 6);
            vi_clear_count();
            vi_enter_insert();
            return true;
        }

        // unrecognised two-key sequence — discard and return to normal
        vi_clear_count();
        return true;
    }

    //-------------------------------------------------------------------------
    // Shift keys
    //-------------------------------------------------------------------------
    if (vk == '4') {
        // $ (shift+4) — end of line
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            static key_event_t seq[] = {
                { VK_SHIFT, 0, KEYEVENTF_KEYUP },  // release shift first
                { VK_END,   0, 0               },
                { VK_END,   0, KEYEVENTF_KEYUP },
            };
            output_sequence(seq, 3);
            return true;
        }
        // if shift not held — fall through to keymap lookup for bare '4'
    }

    //--------------------------------------------------------------------------
    // Get count for n<command>
    //--------------------------------------------------------------------------

    // accumulate digit prefix (1-9 to start, 0-9 thereafter)
    // NOTE: '0' on its own maps to Home, so only treat it as
    // a digit if we already have digits buffered
    bool is_digit = (vk >= '1' && vk <= '9') ||
                    (vk == '0' && g_state.digit_len > 0);
    if (is_digit) {
        if (g_state.digit_len < (int)(sizeof(g_state.digit_buf) - 1)) {
            g_state.digit_buf[g_state.digit_len++] = (char)('0' + (vk - '0'));
        }
        return true;
    }

    int count = vi_get_count();
    vi_clear_count();

    // keys that enter operator-pending mode
    if (vk == 'D' || vk == 'Y' || vk == 'C') {
        g_state.mode        = MODE_OPERATOR_PENDING;
        g_state.operator_vk = vk;
        return true;
    }

    //-------------------------------------------------------------------------
    // Mode switching keys
    //-------------------------------------------------------------------------
    if (vk == 'I') {
        // i — enter insert, same as CAPS LOCK but included for familiarity
        vi_enter_insert();
        return true;
    }

    if (vk == 'A') {
        // a — enter insert after cursor (move right first)
        static key_event_t seq[] = {
            { VK_RIGHT, 0, 0               },
            { VK_RIGHT, 0, KEYEVENTF_KEYUP },
        };
        output_sequence(seq, 2);
        vi_enter_insert();
        return true;
    }

    if (vk == 'O') {
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            // O — open new line above (Home, Enter, Up, indent)
            static key_event_t seq[] = {
                { VK_SHIFT,  0, KEYEVENTF_KEYUP },  // Release shift first
                { VK_HOME,   0, 0               },
                { VK_HOME,   0, KEYEVENTF_KEYUP },
                { VK_RETURN, 0, 0               },
                { VK_RETURN, 0, KEYEVENTF_KEYUP },
                { VK_UP,     0, 0               },
                { VK_UP,     0, KEYEVENTF_KEYUP },
            };
            output_sequence(seq, 6);
        } else {
            // o — open new line below (End, Enter)
            static key_event_t seq[] = {
                { VK_END,    0, 0               },
                { VK_END,    0, KEYEVENTF_KEYUP },
                { VK_RETURN, 0, 0               },
                { VK_RETURN, 0, KEYEVENTF_KEYUP },
            };
            output_sequence(seq, 4);
        }
        vi_enter_insert();
        return true;
    }

    //-------------------------------------------------------------------------
    // Handle the keys
    //-------------------------------------------------------------------------
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
            default:
                break;
        }
    }

    return true;
}