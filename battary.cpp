#include <iostream>
#include <windows.h>
#include <winbase.h>
#include <winioctl.h>

#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

NOTIFYICONDATA g_notifyIconData;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MyAppClass"; 
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, "MyApp", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    g_notifyIconData.hWnd = hwnd;
    g_notifyIconData.uID = TRAY_ICON_ID;
    g_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_notifyIconData.uCallbackMessage = WM_TRAYICON;
    g_notifyIconData.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    lstrcpy(g_notifyIconData.szTip, "My App"); 

    Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_TRAYICON:
            switch (lParam) {
                case WM_RBUTTONDOWN:
                    POINT pt;
                    GetCursorPos(&pt);
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenu(hMenu, MF_STRING, 1, "Button 1");
                    AppendMenu(hMenu, MF_STRING, 2, "Button 2");
                    AppendMenu(hMenu, MF_STRING, 3, "Exit");
                    SetForegroundWindow(hwnd);
                    UINT clicked = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
                    if (clicked == 1) {
                    //------------------------------------------
                        SYSTEM_POWER_STATUS status;
                        if (GetSystemPowerStatus(&status)) {
                            if (status.ACLineStatus == 1) {
                                MessageBox(hwnd, "Charging", "Charging", MB_ICONINFORMATION);
                             } else {
    int myVariable = static_cast<int>(status.BatteryLifePercent);
    std::string message = "Battery: " + std::to_string(myVariable) + "%";
    MessageBox(hwnd, message.c_str(), "%", MB_ICONINFORMATION);
                             }
                         } else {
                             MessageBox(hwnd, "Something went wrong", "Error", MB_ICONINFORMATION);
                         }
                    //------------------------------------------
                    } else if (clicked == 2) {
                        MessageBox(hwnd, "Button 2 pressed1212!", "Message1212", MB_ICONINFORMATION);
                    } else if (clicked == 3) {
                        PostQuitMessage(0);
                    }
                    DestroyMenu(hMenu);
                    break;
            }
            break;

        
        case WM_COMMAND:
        //-----------кусок кода который я не понял зачем нужен(работает и без него)---------------------
            /*if (LOWORD(wParam) == 1) {
                MessageBox(hwnd, "Button 1 pressed!", "Message", MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == 2) {
                MessageBox(hwnd, "Button 2 pressed!", "Message", MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == 3) {
                PostQuitMessage(0);
            }*/
            break;
        //-----------------------------------------------------------------------------------------------
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
