# Vimdows User Guide

## Overview

Vimdows runs in the background and intercepts your keystrokes before they reach
any application. It operates in two modes, similar to the vi text editor:

- **Insert mode** — your keyboard behaves normally, it will type stuff yay
- **Normal mode** — keys are remapped to vi-style navigation and editing actions, on a best effort basis

## Switching Modes

| Key | Action |
|---|---|
| `Caps Lock` | Toggle between insert and normal mode |
| `i` | Enter insert mode at cursor |
| `a` | Enter insert mode after cursor |
| `o` | Enter insert mode on a new line below |
| `O` | Enter insert mode on a new line above |

Note: 'c' will change mode as well, covered later on under "Editing".

## Keybindings

All of the following bindings are active in **normal mode** only. In insert
mode, every key behaves as normal (except CAPS LOCK of course, no one likes that key right).

### Navigation

| Key | Action |
|---|---|
| `h` | Move left |
| `j` | Move down |
| `k` | Move up |
| `l` | Move right |
| `w` | Jump to next word |
| `b` | Jump to previous word |
| `0` | Move to start of line |
| `$` | Move to end of line |

### Editing

| Key | Action |
|---|---|
| `x` | Delete character under cursor |
| `p` | Paste |
| `u` | Undo |
| `Y` | Copy highlighted text |
| `D` | Cut highlighted text |
| `C` | Cut highlighted text then enter insert mode |

Note: 
- Paste is "ctrl + v" backend, so it will behave like "ctrl + v" as opposed to the Vi pasting behaviour. 
- Capital 'Y', 'D' and 'C' are used instead of the lowercase keys as the lowercase keys will wait for a
  subsequent operator (see below). This does not emulate Vi fully as there is no visual mode implementation yet.

### Operators

Operators are two-key combinations. Press the operator key first, then the
motion or target key.

| Sequence | Action |
|---|---|
| `dd` | Delete current line |
| `yy` | Yank (copy) current line |
| `cc` | Change current line (delete and enter insert mode) |
| `dw` | Delete to next word boundary |
| `yw` | Yank to next word boundary |
| `cw` | Change to next word boundary |
| `d$` | Delete to end of line |
| `y$` | Yank to end of line |
| `c$` | Change to end of line |

### Count Prefix

Most normal mode actions can be prefixed with a number to repeat them. Type
the number before the action:

| Example | Action |
|---|---|
| `5j` | Move down 5 lines |
| `3w` | Jump forward 3 words |
| `2dd` | Delete 2 lines |

## Starting Automatically

To have Vimdows start on login without needing to run it manually each time,
register it as a Windows Task Scheduler task:

1. Open Task Scheduler (`Win+R` → `taskschd.msc`)
2. Click **Create Task**
3. Under **General**, check **Run with highest privileges**
4. Under **Triggers**, add a new trigger set to **At log on**
5. Under **Actions**, point it at your `Vimdows.exe`
6. Under **Conditions**, uncheck **Start only if on AC power**

## Known Limitations

- `yy` and other yank operations leave the line deselected at the start of the
  line rather than leaving the cursor at its original position, as vi would.
- `HOME` in some Microsoft Office applications at the start of a bulleted line
  may select the bullet point rather than moving the cursor, affecting `dd`,
  `yy`, and `cc`.
- Yanked text is stored in the Windows clipboard, which is shared with the rest
  of the OS. Copying anything externally (e.g. `Ctrl+C` in another app) will
  overwrite the Vimdows yank buffer.
- No command mode emulation yet, I will try my best to do this if time permits