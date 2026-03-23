#include <windows.h>
#include <stdio.h>
#include "keymap.h"
#include "output.h"
#include "hook.h"

//============================================================================
// Console Control Handler
//============================================================================

static BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        hook_uninstall(); // releases held keys and removes the hook cleanly
        ExitProcess(0);
    }
    return TRUE;
}

//============================================================================
// Entry point
//============================================================================

int main(void) {
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    output_init();
    keymap_init();

    if (!hook_install()) {
        fprintf(stderr, "Failed to install keyboard hook (error %lu)\n", GetLastError());
        return 1;
    }

    printf("Vimdows running. Press Ctrl+C to exit.\n");

    // the message loop is required to keep the low-level hook alive
    // WH_KEYBOARD_LL callbacks are dispatched via the message queue of
    // the thread that called SetWindowsHookEx
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    hook_uninstall();
    return 0;
}