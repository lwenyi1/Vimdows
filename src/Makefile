CC      = gcc
TARGET  = vimdows.exe
SRCS    = main.c hook.c keymap.c vi.c output.c
LIBS    = -luser32

CFLAGS_DEBUG   = -g -Wall -Wextra -DDEBUG
CFLAGS_RELEASE = -O2 -Wall -Wextra
CFLAGS_HIDDEN = -O2 -Wall -Wextra -mwindows 

.PHONY: all debug release clean

all: release

debug:
	$(CC) $(CFLAGS_DEBUG) -o $(TARGET) $(SRCS) $(LIBS)

release:
	$(CC) $(CFLAGS_RELEASE) -o $(TARGET) $(SRCS) $(LIBS)

hidden:
	$(CC) $(CFLAGS_HIDDEN) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	del /Q $(TARGET) 2>nul || true
