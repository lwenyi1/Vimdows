# Vimdows Programmer Guide

## Overview

Vimdows intercepts keystrokes at the Windows low-level hook layer, processes
them through a vi state machine, and emits remapped keystrokes back into the
input stream via the Windows `SendInput` API. It is written in C using only
the Win32 API with no third-party dependencies.

## File Structure

| File | Purpose |
|---|---|
| `main.c` | Entry point, message loop, clean exit handling |
| `hook.c` | Low-level hook callback, filters injected events |
| `vi.c` | vi state machine — modes, operators, count prefix, complex sequences |
| `keymap.c` | Static key remapping tables and layer stack |
| `output.c` | `SendInput` wrappers, sequence emission, stuck-key cleanup |
| `keymap.h` | Shared types — `vi_mode_t`, `key_action_t`, `layer_t`, global state |
| `output.h` | Output function declarations |
| `vi.h` | vi function declarations |
| `hook.h` | Hook function declarations |

## The Pipeline

A keystroke travels through three stages from physical key press to final output.

### Stage 1 — Interception (`hook.c`)

`SetWindowsHookEx(WH_KEYBOARD_LL)` is called in `main.c` to install a
system-wide low-level keyboard hook. This registers `LowLevelKeyboardProc` as
the callback that Windows will invoke for every keystroke on the system, before
it is delivered to any application.

The callback receives:
- `wParam` — the event type (`WM_KEYDOWN`, `WM_KEYUP`, `WM_SYSKEYDOWN`,
  `WM_SYSKEYUP`)
- `lParam` — a pointer to a `KBDLLHOOKSTRUCT` containing the virtual key code
  (`vkCode`) and flags

The first thing the callback does is check the `LLKHF_INJECTED` flag:
```c
if (kb->flags & LLKHF_INJECTED) {
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}
```

This passes through any keystroke that was injected by Vimdows itself via
`SendInput`, preventing infinite loops where our own output re-enters the hook.

For all other keystrokes, the event is handed to `vi_process_key`. If that
function returns `true` (the key was consumed and handled), the hook returns `1`
to swallow the event — the application never sees it. If it returns `false`, the
hook calls `CallNextHookEx` to pass the event through normally.

The message loop in `main.c` is what keeps the hook alive. `WH_KEYBOARD_LL`
callbacks are dispatched via the message queue of the thread that installed the
hook, so that thread must continuously pump messages:
```c
MSG msg;
while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

### Stage 2 — Processing (`vi.c` and `keymap.c`)

`vi_process_key` is the core of the program. It receives the virtual key code
and a boolean indicating whether it is a key-down or key-up event. Key-up events
are ignored — all logic is driven by key-down events only.

Processing follows this order:

1. **Caps Lock** — checked unconditionally first. Toggles between insert and
   normal mode regardless of any other state.

2. **Insert mode passthrough** — if the current mode is `MODE_INSERT`, return
   `false` immediately so all keys pass through unchanged.

3. **Shift-state keys** — keys whose behaviour depends on whether shift is
   physically held (currently `$` via `Shift+4`) are checked before digit
   accumulation to prevent `4` being misidentified as a count prefix.

4. **Digit accumulation** — if the key is a digit (`1`–`9`, or `0` when digits
   are already buffered), it is appended to the count prefix buffer and consumed.
   `0` alone is not treated as a digit because it maps to `Home`.

5. **Operator-pending mode** — if the state is `MODE_OPERATOR_PENDING`, the
   incoming key is the second half of a two-key sequence (`dd`, `dw`, `y$`
   etc.). The operator and motion are matched and the corresponding key sequence
   is emitted. Unrecognised combinations are discarded and the mode resets to
   normal.

6. **Count retrieval** — the accumulated count prefix is read and cleared. All
   subsequent actions use this count to determine how many times to repeat.

7. **Operator triggers** — `d`, `y`, and `c` set `MODE_OPERATOR_PENDING` and
   store the operator key, then return. The next keypress will be caught by
   step 5 above.

8. **Mode-switching keys** — `i`, `a`, `o`, `O` switch to insert mode, some
   with a preparatory sequence (e.g. `a` moves the cursor right first).

9. **Keymap lookup** — all remaining keys are looked up in the active layer via
   `keymap_lookup`. If no binding is found, the key passes through. If a binding
   exists, the corresponding action is executed.

#### The Layer Stack

The keymap is organised as a stack of layers. Each layer is an array of
`key_action_t` indexed directly by virtual key code (0x00–0xFF), giving O(1)
lookup. `keymap_lookup` walks the stack from top to bottom and returns the first
non-`ACTION_NONE` binding it finds, allowing upper layers to override lower ones.

Currently two layers are defined:

- **Layer 0 — insert** — entirely passthrough (`ACTION_NONE` for all keys)
- **Layer 1 — normal** — remaps for navigation, word movement, editing

Mode switches push and pop layers to keep the active layer in sync with the vi
mode. `ACTION_CONSUMED` entries are registered in the normal layer for keys that
are handled entirely in `vi.c` (operators, mode switches) — this prevents them
reaching the `return false` passthrough at the end of `vi_process_key`.

### Stage 3 — Output (`output.c`)

All output goes through `SendInput`, which injects events directly into the
Windows input stream. Three functions are provided:

- `output_tap(vk)` — press and release a single key
- `output_key(vk, keydown)` — press or release a single key, tracking held state
- `output_sequence(events, count)` — send an array of `key_event_t` structs in
  a single `SendInput` call, guaranteeing they arrive atomically with no other
  events interleaved

Sequences are used for any action requiring multiple keystrokes — modifier
combinations (`Ctrl+Right` for word jump), selections (`Shift+End`), and
multi-step operations (`dd` = Home → Shift+End → Ctrl+C → Delete).

For sequences involving clipboard operations, a `Sleep(20)` is inserted between
the copy and the subsequent action. This is necessary because the Windows
clipboard write is asynchronous — without the delay, a delete or paste
immediately following a copy may operate on stale clipboard contents.

`output_release_all` tracks which virtual keys are currently held and releases
them all. This is called by `hook_uninstall` on exit to prevent modifier keys
becoming stuck when the hook is removed mid-keystroke.

## Extending the Program

### Adding a new normal-mode keybinding

For a simple remap, add a `set_key` or sequence entry in `setup_normal_layer`
in `keymap.c`:
```c
set_key(&layer_normal, 'G', VK_END);  // G = Ctrl+End (document end)
```

For anything requiring state awareness (mode switches, shift detection, operator
pending), add a handler directly in `vi_process_key` in `vi.c` and register the
key as `ACTION_CONSUMED` in `keymap.c`.

### Adding a new operator-motion combination

Add a new `if (op == 'X' && vk == 'Y')` case inside the
`MODE_OPERATOR_PENDING` block in `vi_process_key`. Follow the pattern of the
existing cases — emit the selection sequence, then the action sequence, then
call `vi_clear_count` and return `true`.

### Adding a new mode

Add a new value to `vi_mode_t` in `keymap.h`, a new layer definition in
`keymap.c`, and corresponding `vi_enter_*` and handling logic in `vi.c`.
Visual mode would be the natural next addition, using shift-extended motion
sequences to build a selection before an operator is applied.
