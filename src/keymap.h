#ifndef KEYMAP_H
#define KEYMAP_H

#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

// ─── Modes ───────────────────────────────────────────────────────────────────

typedef enum {
    MODE_INSERT,
    MODE_NORMAL,
    MODE_OPERATOR_PENDING
} vi_mode_t;

// ─── Action Types ────────────────────────────────────────────────────────────

typedef enum {
    ACTION_NONE,        // do nothing / passthrough
    ACTION_KEY,         // emit a single key
    ACTION_SEQUENCE,    // emit a sequence of keys
    ACTION_FUNCTION,    // call a C function
} action_type_t;

// a single key event (one entry in a sequence)
typedef struct {
    WORD    vk;         // virtual key code
    WORD    scan;       // scan code (0 = derive from vk)
    DWORD   flags;      // KEYEVENTF_* flags (0 = keydown, KEYEVENTF_KEYUP = up)
} key_event_t;

// the action to perform when a key is triggered
typedef struct {
    action_type_t   type;
    union {
        WORD        vk;                 // ACTION_KEY
        struct {
            key_event_t *events;        // ACTION_SEQUENCE
            int          count;
        } sequence;
        void        (*fn)(void);        // ACTION_FUNCTION
        vi_mode_t   mode;               // ACTION_MODE_SWITCH
    };
} key_action_t;

// ─── Layers ──────────────────────────────────────────────────────────────────

#define MAX_LAYERS      8
#define MAX_KEYS        256     // one slot per possible vkCode (0x00–0xFF)

typedef struct {
    const char  *name;
    key_action_t keys[MAX_KEYS]; // indexed directly by vkCode
} layer_t;

// ─── Layer Stack ─────────────────────────────────────────────────────────────

typedef struct {
    layer_t    *layers[MAX_LAYERS];
    int         top;                // index of the topmost active layer (-1 = empty)
} layer_stack_t;

// ─── Global State ────────────────────────────────────────────────────────────

typedef struct {
    vi_mode_t       mode;
    layer_stack_t   stack;
    WORD            operator_vk;    // pending operator key (e.g. 'd', 'c', 'y')
    char            digit_buf[8];   // count prefix buffer e.g. "12" for 12j
    int             digit_len;
} vi_state_t;

extern vi_state_t g_state;

// ─── Function Declarations ───────────────────────────────────────────────────

void        keymap_init(void);
key_action_t *keymap_lookup(WORD vk);
void        layer_push(int layer_index);
void        layer_pop(void);

#endif // KEYMAP_H