CC      = gcc
TARGET  = ../output/vimdows.exe
SRCS    = main.c hook.c keymap.c vi.c output.c
RC      = vimdows.rc
RES     = ../output/vimdows.res
LIBS    = -luser32 -lshell32

CFLAGS_DEBUG   = -g -Wall -Wextra -DDEBUG
CFLAGS_RELEASE = -O2 -Wall -Wextra -mwindows

.PHONY: all debug release clean

all: release

debug:
	if not exist "../output" mkdir "..\output"
	windres $(RC) -O coff -o $(RES)
	$(CC) $(CFLAGS_DEBUG) -o $(TARGET) $(SRCS) $(RES) $(LIBS)

release:
	if not exist "../output" mkdir "..\output"
	windres $(RC) -O coff -o $(RES)
	$(CC) $(CFLAGS_RELEASE) -o $(TARGET) $(SRCS) $(RES) $(LIBS)

clean:
	del /Q $(TARGET) $(RES) 2>nul || true
