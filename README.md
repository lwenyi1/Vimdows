# Vimdows

## Acknowledgements
This project was inspired by [KMonad](https://github.com/kmonad/kmonad?tab=readme-ov-file).

## Introduction
A lightweight Windows keyboard remapper that (somewhat) emulates vi's modal 
editing across the entire OS. Started this pet project because I'm too lazy 
to carry a mouse around and strongly dislike using my trackpad.

## Dependencies

- **Windows 10/11**
- **GCC** — any MinGW-w64 distribution (e.g. via [Scoop](https://scoop.sh): `scoop install gcc`)
- No third-party libraries — only the Windows SDK (`user32`) is required

## Building
The program can be compiled using:

```bash
gcc -o vimdows.exe main.c hook.c keymap.c vi.c output.c -luser32
```

Might add a Makefile at some point.

## Running

vi-kbd must be run as Administrator — the low-level hook requires elevated
privileges to intercept keystrokes system-wide. It can be run with:

```bash
./vimdows.exe
```

For full keybinding reference and usage instructions, see the **User Guide**.
For an explanation of the architecture and how to extend the program, see the
**Programmer Guide**.
BOTH OF WHICH ARE NONEXISTENT AT THE MOMENT AND I WILL MAKE AT SOME POINT PROMISE