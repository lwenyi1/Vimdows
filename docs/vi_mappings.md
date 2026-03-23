# Mappings from vi to Windows equivalents

This table also kind of acts as a TODO list for me.

| vi Command | Windows/GUI Equivalent | Use / Description |
| --- | --- | --- |
| h, j, k, l | Arrow Keys | Move cursor: Left, Down, Up, Right |
| w | Ctrl + Right Arrow | Move forward one word |
| b | Ctrl + Left Arrow | Move backward one word |
| e | WIP | Move to the end of the current word |
| 0 (zero) | Home | Move to the beginning of the current line |
| ^ | WIP | Move to the first non-blank character of the line |
| $ | End | Move to the end of the current line |
| G | Ctrl + End | Move to the last line of the file |
| 1G | Ctrl + Home | Move to the first line of the file |
| H | WIP | Move to the top line of the screen |
| M | WIP | Move to the middle line of the screen |
| L | WIP | Move to the bottom line of the screen |
| i | (insert) | Insert text before the cursor |
| a | Right, (insert) | Append text after the cursor |
| I | WIP | Insert text at the beginning of the line |
| A | WIP | Append text at the end of the line |
| o | End, return, (insert) | Open a new line below and enter Insert Mode |
| O | Home, return, (insert) | Open a new line above and enter Insert Mode |
| x | Delete | Delete the character under the cursor |
| X | WIP | Delete the character before the cursor |
| dd | (select line), delete | Delete (cut) the entire current line |
| dw | Ctrl + Delete | Delete from cursor to the start of the next word |
| d$ | Ctrl + Shift + End + Del | Delete from cursor to the end of the line |
| yy | (select line) + Ctrl + C | Yank (copy) the current line |
| yw | (select word), Ctrl + C | Yank (copy) the current word |
| p | Ctrl + V | Paste (put) after the cursor |
| P | WIP | Paste (put) before the cursor |
| r | WIP | Replace a single character (overtype) |
| R | WIP | Enter replace Mode (overtype text) |
| u | Ctrl + Z | Undo last change |
| U | WIP | Undo all changes on the current line |
| . | WIP | Repeat the last editing command |
| / | Ctrl + F | Search forward for a pattern |
| ? | (1) | Search backward for a pattern |
| n | WIP | Repeat search in the same direction |
| N | WIP | Repeat search in the opposite direction |
| Ctrl + f | WIP | Scroll down one full screen |
| Ctrl + b | WIP | Scroll up one full screen |
| Esc | (2) | Exit Insert Mode and return to Command Mode |

(1) Effectively same as / on Windows  
(2) Contemplating whether adding this would break some normal stuff because
escape is a rather important key.
