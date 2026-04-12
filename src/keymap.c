/**
 * @file keymap.c
 * @brief Key mapping and state tracking
 * 
 * Handles simple key remapping and state tracking for vi mode.
 * The key mappings here are those which do not need awareness of
 * their vi state and are simple translations.
 */

#include <string.h>
#include "keymap.h"
#include "output.h"

//=============================================================================
// Globals
//=============================================================================

vi_state_t g_state = {
    .mode        = MODE_INSERT,
    .stack       = { .top = -1 },
    .operator_vk = 0,
    .digit_buf   = {0},
    .digit_len   = 0
};

//=============================================================================
// Layer definitions
//
// layers are defined as static structs and registered in keymap_init()
// keys not explicitly set default to ACTION_NONE (zero-initialised)
//=============================================================================

static layer_t layer_insert;
static layer_t layer_normal;

static layer_t *g_layer_registry[MAX_LAYERS] = {0};
static int      g_layer_count                = 0;

void layer_push(int layer_index) {
    if (g_state.stack.top >= MAX_LAYERS - 1) return;
    if (layer_index < 0 || layer_index >= g_layer_count) return;
    g_state.stack.layers[++g_state.stack.top] = g_layer_registry[layer_index];
}

void layer_pop(void) {
    if (g_state.stack.top < 0) return;
    g_state.stack.top--;
}

//=============================================================================
// Key lookup
//=============================================================================

// walk the layer stack from top to bottom and return the first
// non-ACTION_NONE binding for this vkCode, or NULL for passthrough
key_action_t *keymap_lookup(WORD vk) {
    for (int i = g_state.stack.top; i >= 0; i--) {
        key_action_t *action = &g_state.stack.layers[i]->keys[vk];
        if (action->type != ACTION_NONE) {
            return action;
        }
    }
    return NULL; // passthrough
}

static void set_nop(layer_t *layer, WORD vk) {
    layer->keys[vk].type = ACTION_NOP;
}

static void set_key(layer_t *layer, WORD from_vk, WORD to_vk) {
    layer->keys[from_vk].type = ACTION_KEY;
    layer->keys[from_vk].vk   = to_vk;
}

static void set_fn(layer_t *layer, WORD from_vk, void (*fn)(void)) {
    layer->keys[from_vk].type = ACTION_FUNCTION;
    layer->keys[from_vk].fn   = fn;
}

//=============================================================================
// Layer setup
//=============================================================================

static void setup_insert_layer(void) {
    layer_insert.name = "insert";
    memset(layer_insert.keys, 0, sizeof(layer_insert.keys));
    // insert layer is mostly passthrough (ACTION_NONE)
    // only caps needs to be intercepted to switch mode
    // (handled directly in vi.c, not here)
}

static void setup_normal_layer(void) {
    layer_normal.name = "normal";
    memset(layer_normal.keys, 0, sizeof(layer_normal.keys));

    //-------------------------------------------------------------------------
    // Movement
    //-------------------------------------------------------------------------

    set_key(&layer_normal, 'H', VK_LEFT);
    set_key(&layer_normal, 'J', VK_DOWN);
    set_key(&layer_normal, 'K', VK_UP);
    set_key(&layer_normal, 'L', VK_RIGHT);
    set_key(&layer_normal, '0', VK_HOME);
    set_key(&layer_normal, 'X', VK_DELETE);

    //-------------------------------------------------------------------------
    // Word movement
    //-------------------------------------------------------------------------
    // w = Ctrl+Right,  b = Ctrl+Left
    // these need modifier wrapping — handled as sequences
    static key_event_t w_seq[] = {
        { VK_CONTROL, 0, 0               },
        { VK_RIGHT,   0, 0               },
        { VK_RIGHT,   0, KEYEVENTF_KEYUP },
        { VK_CONTROL, 0, KEYEVENTF_KEYUP },
    };
    static key_event_t b_seq[] = {
        { VK_CONTROL, 0, 0               },
        { VK_LEFT,    0, 0               },
        { VK_LEFT,    0, KEYEVENTF_KEYUP },
        { VK_CONTROL, 0, KEYEVENTF_KEYUP },
    };
    layer_normal.keys['W'].type           = ACTION_SEQUENCE;
    layer_normal.keys['W'].sequence.events = w_seq;
    layer_normal.keys['W'].sequence.count  = 4;
    layer_normal.keys['B'].type           = ACTION_SEQUENCE;
    layer_normal.keys['B'].sequence.events = b_seq;
    layer_normal.keys['B'].sequence.count  = 4;

    //-------------------------------------------------------------------------
    // Misc editing shortcuts
    //-------------------------------------------------------------------------

    set_key(&layer_normal, 'P', 'V');
    set_key(&layer_normal, 'U', 'Z');

    static key_event_t undo_seq[] = {
        { VK_CONTROL, 0, 0               },
        { 'Z',        0, 0               },
        { 'Z',        0, KEYEVENTF_KEYUP },
        { VK_CONTROL, 0, KEYEVENTF_KEYUP },
    };
    static key_event_t paste_seq[] = {
        { VK_CONTROL, 0, 0               },
        { 'V',        0, 0               },
        { 'V',        0, KEYEVENTF_KEYUP },
        { VK_CONTROL, 0, KEYEVENTF_KEYUP },
    };
    layer_normal.keys['U'].type            = ACTION_SEQUENCE;
    layer_normal.keys['U'].sequence.events = undo_seq;
    layer_normal.keys['U'].sequence.count  = 4;

    layer_normal.keys['P'].type            = ACTION_SEQUENCE;
    layer_normal.keys['P'].sequence.events = paste_seq;
    layer_normal.keys['P'].sequence.count  = 4;

    //-------------------------------------------------------------------------
    // Unbound keys, do nothing for now
    //
    // These are keys I keep accidentally pressing and want them to not do
    // anything >:(
    //-------------------------------------------------------------------------
    set_nop(&layer_normal, 'Q');
    set_nop(&layer_normal, 'E');
    set_nop(&layer_normal, 'R');
    set_nop(&layer_normal, 'N');
    set_nop(&layer_normal, 'M');
}

//=============================================================================
// Init
//=============================================================================

void keymap_init(void) {
    setup_insert_layer();
    setup_normal_layer();

    g_layer_registry[0] = &layer_insert;
    g_layer_registry[1] = &layer_normal;
    g_layer_count       = 2;

    // start on the insert layer
    layer_push(0);
}