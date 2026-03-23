/**
 * @file hook.c
 * @brief Low-level keyboard hook
 * 
 * Installs and uninstalls the low-level keyboard hook.
 */

#include <windows.h>
#include <stdbool.h>
#include "keymap.h"
#include "vi.h"
#include "output.h"

static HHOOK g_hook = NULL;

//-------------------------------------------------------------------------
// Low-level keyboard hook
//-------------------------------------------------------------------------

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION) {
        return CallNextHookEx(g_hook, nCode, wParam, lParam);
    }

    KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT *)lParam;

    // ignore injected events — these are keys we emitted ourselves via
    // SendInput, letting them pass through prevents infinite loops
    if (kb->flags & LLKHF_INJECTED) {
        return CallNextHookEx(g_hook, nCode, wParam, lParam);
    }

    bool keydown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
    WORD vk      = (WORD)kb->vkCode;

    // hand off to the vi state machine
    bool consumed = vi_process_key(vk, keydown);

    if (consumed) {
        return 1;  // swallow the event — don't pass to the next hook or app
    }

    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

//-------------------------------------------------------------------------
// Hook management
//-------------------------------------------------------------------------

bool hook_install(void) {
    g_hook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        GetModuleHandle(NULL),
        0   // 0 = system-wide hook
    );
    return g_hook != NULL;
}

void hook_uninstall(void) {
    if (g_hook) {
        output_release_all(); // release any held keys before unhooking
        UnhookWindowsHookEx(g_hook);
        g_hook = NULL;
    }
}
