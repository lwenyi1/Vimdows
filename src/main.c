#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include "keymap.h"
#include "output.h"
#include "hook.h"
#include "resource.h"

#define WM_TRAY_ICON    (WM_USER + 1)
#define TRAY_CLASS_NAME "VimdownsTray"

static NOTIFYICONDATA   g_nid      = {0};
static HWND             g_hwnd     = NULL;
static bool             g_enabled  = true;

//===========================================================================
// Tray icon
//===========================================================================

static void tray_update_tooltip(void) {
    const char *status = g_enabled ? "vimdows [on]" : "vimdows [off]";
    strncpy(g_nid.szTip, status, sizeof(g_nid.szTip) - 1);
    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

static void tray_add(HWND hwnd) {
    g_nid.cbSize           = sizeof(NOTIFYICONDATA);
    g_nid.hWnd             = hwnd;
    g_nid.uID              = 1;
    g_nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY_ICON;
    g_nid.hIcon            = LoadIcon(GetModuleHandle(NULL),
                                      MAKEINTRESOURCE(IDI_TRAY_ICON));
    strncpy(g_nid.szTip, "vimdows [on]", sizeof(g_nid.szTip) - 1);
    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

static void tray_remove(void) {
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
}

static void tray_show_menu(HWND hwnd) {
    HMENU menu  = CreatePopupMenu();
    UINT  flags = MF_BYPOSITION | MF_STRING;

    InsertMenu(menu, 0, flags,
               ID_TRAY_TOGGLE, g_enabled ? "Disable" : "Enable");
    InsertMenu(menu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenu(menu, 2, flags, ID_TRAY_EXIT, "Exit");

    // required before and after TrackPopupMenu when using a tray icon
    SetForegroundWindow(hwnd);

    POINT pt;
    GetCursorPos(&pt);
    TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                   pt.x, pt.y, 0, hwnd, NULL);

    DestroyMenu(menu);
}

//===========================================================================
// Toggle
//===========================================================================

static void toggle_enabled(void) {
    g_enabled = !g_enabled;
    if (g_enabled) {
        hook_install();
    } else {
        hook_uninstall();
    }
    tray_update_tooltip();
}

//===========================================================================
// Windows Proc
//===========================================================================

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
                                 WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TRAY_ICON:
            switch (LOWORD(lParam)) {
                case WM_RBUTTONUP:
                    tray_show_menu(hwnd);
                    break;
                case WM_LBUTTONDBLCLK:
                    toggle_enabled();
                    break;
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_TOGGLE:
                    toggle_enabled();
                    break;
                case ID_TRAY_EXIT:
                    PostQuitMessage(0);
                    break;
            }
            return 0;

        case WM_DESTROY:
            tray_remove();
            hook_uninstall();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

//===========================================================================
// Main
//===========================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    // register a hidden window class to receive tray messages
    WNDCLASSEX wc    = {0};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = TRAY_CLASS_NAME;
    RegisterClassEx(&wc);

    // create the hidden message window — it is never shown
    g_hwnd = CreateWindowEx(0, TRAY_CLASS_NAME, "vimdows",
                             0, 0, 0, 0, 0,
                             HWND_MESSAGE, NULL, hInstance, NULL);

    output_init();
    keymap_init();

    if (!hook_install()) {
        MessageBox(NULL, "Failed to install keyboard hook.",
                   "vimdows", MB_ICONERROR | MB_OK);
        return 1;
    }

    tray_add(g_hwnd);

    // message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
