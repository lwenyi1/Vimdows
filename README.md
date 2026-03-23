# Vimdows

## Acknowledgements
The core engine of this project (low level hook > remap > output key) was inspired 
by [KMonad](https://github.com/kmonad/kmonad?tab=readme-ov-file).

## Introduction
A lightweight Windows keyboard remapper that (somewhat) emulates vi's modal 
editing across the entire OS. I was taking a shower one day and thought "Wouldn't it
be nice if I could use h j k and l while working on Microsoft Word". I also don't really
like using my tr*ckpad.

## Dependencies

- **Windows 10/11**
- **GCC** — any MinGW-w64 distribution (e.g. via [Scoop](https://scoop.sh): `scoop install gcc`)
- No third-party libraries — only the Windows SDK (`user32`) is required

## Building
The program can be compiled using:

```bash
gcc -o vimdows.exe main.c hook.c keymap.c vi.c output.c -luser32
```

## Running

Vimdows must be run as Administrator — the low-level hook requires elevated
privileges to intercept keystrokes system-wide. It can be run with:

```bash
./vimdows.exe
```

For full keybinding reference and usage instructions, see the [**User Guide**](docs/user_guide.md).

For an explanation of the architecture and how to extend the program, see the
[**Programmer Guide**](docs/programmer_guide.md).

For a list of currently implemented Vi keys and their windows equivalent, see [**vi_mappings**](docs/vi_mappings.md).

## Contributing
Juggling work and university at the moment, so this project is really just the bare-bones features of Vi
and was tossed together so I wouldn't have to use my tr*ckpad so often. I think it has the potential to become a 
true "Vi for everything in Windows", so contributions would be greatly appreciated.

### Potential new features:
- The remaining basic Vi keybinds
- More specific editing features, e.g. ciw to "change in word"
- More optimised version of jumping to a line with 'g', current method just goes to the top and down the number of lines.
- Command mode (may need to add some display elements to this program to make this feature usable) 
- Visual mode for selection
- Automated testing for keybinds with a Python script

I can be contacted at liu_wenyi@u.nus.edu (include "Vimdows" in email subject) for more discussion.
