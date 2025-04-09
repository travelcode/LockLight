// LockLight.cpp : 定义应用程序的入口点。
//

#include "pch.h"
#include "LockLight.h"

constexpr auto MAX_LOADSTRING = 100;

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
WCHAR szCapsLockTip[MAX_LOADSTRING];            // Caps Lock提示文本
WCHAR szNumLockTip[MAX_LOADSTRING];             // Num Lock提示文本
WCHAR szScrollLockTip[MAX_LOADSTRING];          // Scroll Lock提示文本
BOOL bDarkMode = FALSE;                     // 深色模式标志
BOOL HideWindow = TRUE;                  // 隐藏窗口标志
HICON g_hCapsIconOn = NULL;
HICON g_hNumIconOn = NULL;
HICON g_hScrollIconOn = NULL;
HHOOK g_hKeyboardHook = NULL;
// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
void SetThemeMode(HWND hWnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
bool IsSystemDarkMode();
void RemoveTrayIcon(HWND hWnd);
void ShowContextMenu(HWND hWnd);
void AddTrayIcon(HWND hWnd, HICON hIcon, UINT uID, LPCWSTR szTip);
void SetTitleBarDarkMode(HWND hWnd, BOOL dark);
void UpdateKeyboardIcons(HWND hWnd);
void RegisterStartLanuch();
void InitializeKeyboardIcons(HINSTANCE hInstance);
void UpdateSingleIcon(HWND hWnd, UINT uID, UINT uMsg,BOOL bState, LPCWSTR szTip);

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"LockLight_7a1f801e-df61-463d-9403-fb4e43042f9f");
    if (hMutex!=NULL&&GetLastError() == ERROR_ALREADY_EXISTS) {
        // 互斥体已存在，说明程序已在运行
        CloseHandle(hMutex);
        return 0;
    }
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LOCKLIGHT, szWindowClass, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_CAPSLOCK_TIP, szCapsLockTip, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_NUMLOCK_TIP, szNumLockTip, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_SCROLLLOCK_TIP, szScrollLockTip, MAX_LOADSTRING);
	// 检查系统是否为深色模式
	bDarkMode = IsSystemDarkMode(); // 检查系统是否为深色模式
    MyRegisterClass(hInstance);
	SetThemeAppProperties(STAP_VALIDBITS); // 设置应用程序主题属性
    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}
void RegisterStartLanuch() {
    // 添加开机启动（当前用户）
    HKEY hKey;
    LONG lResult = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_WRITE,
        &hKey
    );

    if (lResult == ERROR_SUCCESS) {
        WCHAR szPath[MAX_PATH];
        GetModuleFileName(NULL, szPath, MAX_PATH); // 获取当前程序路径
        // 写入注册表
        RegSetValueEx(
            hKey,
            L"LockLight",          // 启动项名称（随意）
            0,
            REG_SZ,
            (BYTE*)szPath,        // 程序完整路径
            (wcslen(szPath) + 1) * sizeof(WCHAR)
        );
        RegCloseKey(hKey);
    }
}
void InitializeKeyboardIcons(HINSTANCE hInstance) {
	// 检查系统是否为深色模式
	bDarkMode = IsSystemDarkMode();
	// 如果是深色模式,加载暗黑模式图标
    if (bDarkMode) {

        g_hCapsIconOn = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CAPSLOCK_DARK));
        g_hNumIconOn = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NUMLOCK_DARK));
        g_hScrollIconOn = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCROLLLOCK_DARK));
	}
    else {
		// 如果是浅色模式,加载亮色模式图标
		g_hCapsIconOn = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CAPSLOCK_WHITE));
		g_hNumIconOn = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NUMLOCK_WHITE));
		g_hScrollIconOn = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCROLLLOCK_WHITE));
    }
}

void UpdateKeyboardIcons(HWND hWnd)
{
   
    // 获取键盘状态
    BOOL bCapsOn = (GetKeyState(VK_CAPITAL) & 0x0001);
    BOOL bNumOn = (GetKeyState(VK_NUMLOCK) & 0x0001);
    BOOL bScrollOn = (GetKeyState(VK_SCROLL) & 0x0001);

    // 更新Caps Lock图标
    UpdateSingleIcon(hWnd, IDI_CAPSLOCK, WM_CAPSLOCK,bCapsOn, szCapsLockTip);

    // 更新Num Lock图标
    UpdateSingleIcon(hWnd, IDI_NUMLOCK, WM_NUMLOCK,bNumOn, szNumLockTip);

    // 更新Scroll Lock图标
    UpdateSingleIcon(hWnd, IDI_SCROLLLOCK,WM_SCROLLLOCK, bScrollOn, szScrollLockTip);
}

void UpdateSingleIcon(HWND hWnd, UINT uID, UINT uMsg,BOOL bState, LPCWSTR szTip)
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = uMsg;

    if (bState)
    {
        // 根据ID设置对应的图标
        switch (uID)
        {
        case IDI_CAPSLOCK:
            nid.hIcon = g_hCapsIconOn;
            break;
        case IDI_NUMLOCK:
            nid.hIcon = g_hNumIconOn;
            break;
        case IDI_SCROLLLOCK:
            nid.hIcon = g_hScrollIconOn;
            break;
        }
        wcsncpy_s(nid.szTip, szTip, _TRUNCATE);

        // 添加或更新图标
        // 先尝试修改，如果失败则添加
        if (!Shell_NotifyIcon(NIM_MODIFY, &nid))
        {
            Shell_NotifyIcon(NIM_ADD, &nid);
        }
        Shell_NotifyIcon(NIM_SETVERSION, &nid);
    }
    else
    {
        // 如果不需要显示图标，则删除
        Shell_NotifyIcon(NIM_DELETE, &nid);
    }
}
bool IsSystemDarkMode() {
    HKEY hKey;
    DWORD value = 1; // 默认值（浅色模式）
    DWORD dataSize = sizeof(DWORD);

    // 打开注册表键
    if (RegOpenKeyEx(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0,
        KEY_READ,
        &hKey
    ) != ERROR_SUCCESS) {
        return false; // 默认返回浅色模式
    }

    // 读取 AppsUseLightTheme 值
    if (RegQueryValueEx(
        hKey,
        L"AppsUseLightTheme",
        NULL,
        NULL,
        (LPBYTE)&value,
        &dataSize
    ) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false; // 默认返回浅色模式
    }

    RegCloseKey(hKey);
    return (value == 0); // 0=深色模式, 1=浅色模式
}
// 添加托盘图标
void AddTrayIcon(HWND hWnd, HICON hIcon, UINT uID = 1, LPCWSTR szTip = L"") {
    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hIcon;
    //nid.uVersion = NOTIFYICON_VERSION_4;AA4758
    if (szTip && *szTip) {
        wcsncpy_s(nid.szTip, szTip, _TRUNCATE);
    }

    Shell_NotifyIcon(NIM_ADD, &nid);
    Shell_NotifyIcon(NIM_SETVERSION, &nid);
}
void RemoveTrayIcon(HWND hWnd) {
    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = hWnd;
    nid.uID = 1;

    Shell_NotifyIcon(NIM_DELETE, &nid);
}
// 处理托盘图标消息
void ShowContextMenu(HWND hWnd) {
    HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_COMMONMENU));
    HMENU hSubMenu = GetSubMenu(hMenu, 0); // 获取第一个弹出菜单
    // 设置暗黑模式主题
    SetWindowTheme((HWND)hSubMenu, bDarkMode ? L"DarkMode_Explorer" : L"Explorer", NULL);
    // 检测按键状态并设置菜单项
    UINT capsState = GetKeyState(VK_CAPITAL);
    UINT numState = GetKeyState(VK_NUMLOCK);
    UINT scrollState = GetKeyState(VK_SCROLL);

    // 设置菜单项选中状态
    CheckMenuItem(hSubMenu, ID_CAPLOCK_TRAY, (capsState & 0x0001) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hSubMenu, ID_NUMLOCK_TRAY, (numState & 0x0001) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hSubMenu, ID_SCROLLLOCK_TRAY, (scrollState & 0x0001) ? MF_CHECKED : MF_UNCHECKED);
    POINT pt;
    GetCursorPos(&pt);  // 获取鼠标位置

    // 显示菜单
    TrackPopupMenu(
        hSubMenu,
        TPM_BOTTOMALIGN | TPM_LEFTALIGN,
        pt.x, pt.y,
        0,
        hWnd,
        NULL
    );

    DestroyMenu(hMenu);
}
//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOCKLIGHT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_EX_TOOLWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   // 注册原始输入设备
   RAWINPUTDEVICE rid{};
   rid.usUsagePage = 0x01;          // 通用桌面控制
   rid.usUsage = 0x06;              // 键盘
   rid.dwFlags = RIDEV_INPUTSINK;   // 即使窗口不活动也接收输入
   rid.hwndTarget = hWnd;

   if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
   {
	   MessageBox(NULL, L"Failed to register raw input device!", L"Error", MB_OK | MB_ICONERROR);
	   return FALSE;
   }
   SetThemeMode(hWnd);
   RegisterStartLanuch();
   if (HideWindow) {
	   // 隐藏窗口
	   ShowWindow(hWnd, SW_HIDE);
   }
   else {
	   // 显示窗口
	   ShowWindow(hWnd, nCmdShow);
   }
   UpdateWindow(hWnd);

   return TRUE;
}

// 设置窗口的深色模式
void SetTitleBarDarkMode(HWND hWnd, BOOL dark) {
    BOOL value = dark;
    DwmSetWindowAttribute(
        hWnd,
        DWMWA_USE_IMMERSIVE_DARK_MODE,  // 20 (Win11) 或 19 (Win10 1809+)
        &value,
        sizeof(value)
    );
}

// 设置主题模式
void SetThemeMode(HWND hWnd)
{
    if (bDarkMode) {
        // 深色模式下的窗口背景颜色
        SetWindowTheme(hWnd, L"DarkMode_Explorer", NULL);  // Win11 深色
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(32, 32, 32)));
		SetTitleBarDarkMode(hWnd, TRUE);  // 设置标题栏为深色模式
    }
    else
    {
        // 浅色模式下的窗口背景颜色
        SetWindowTheme(hWnd, L"Explorer", NULL);  // Win11 浅色
		SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(255, 255, 255)));
		SetTitleBarDarkMode(hWnd, FALSE);  // 设置标题栏为浅色模式
    }
    RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
}
// 键盘钩子回调函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        HWND hWnd = FindWindow(szWindowClass, NULL);
        if (hWnd)
        {
            PostMessage(hWnd, WM_UPDATEICONS, 0, 0);
        }
    }
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static NOTIFYICONDATA nidCaps = { 0 };
    static NOTIFYICONDATA nidNum = { 0 };
    static NOTIFYICONDATA nidScroll = { 0 };
    switch (message)
    {
	case WM_CREATE:
	{
        // 初始化图标
        InitializeKeyboardIcons(hInst);
		// 添加托盘图标
		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LOCKLIGHT));
		AddTrayIcon(hWnd, hIcon, IDI_TRAY,szTitle);
        // 立即更新一次图标状态
        UpdateKeyboardIcons(hWnd);
        // 安装键盘钩子
		g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInst, 0);
		if (!g_hKeyboardHook)
		{
			MessageBox(hWnd, L"Failed to install keyboard hook!", L"Error", MB_OK | MB_ICONERROR);
		}
	}
	break;
    case WM_UPDATEICONS:
    {
        // 更新图标状态
        UpdateKeyboardIcons(hWnd);
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 解析菜单选择:
        switch (wmId)
        {
        case ID_CAPLOCK_TRAY:
        {
            // 切换Caps Lock状态
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = VK_CAPITAL;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));

            // 更新图标和菜单状态
            PostMessage(hWnd, WM_UPDATEICONS, 0, 0);
            break;
        }
        case ID_NUMLOCK_TRAY:
        {
            // 切换Num Lock状态
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = VK_NUMLOCK;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));

            // 更新图标和菜单状态
            PostMessage(hWnd, WM_UPDATEICONS, 0, 0);
            break;
        }
        case ID_SCROLLLOCK_TRAY:
        {
            // 切换Scroll Lock状态
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = VK_SCROLL;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));

            // 更新图标和菜单状态
            PostMessage(hWnd, WM_UPDATEICONS, 0, 0);
            break;
        }
        case ID_EXIT_TRAY:
            // 退出程序
            PostMessage(hWnd, WM_DESTROY, 0, 0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
        break;

    case WM_SETTINGCHANGE:
        if (lParam && wcscmp((LPCWSTR)lParam, L"ImmersiveColorSet") == 0) {
            // 主题变化，重新检测
            bDarkMode = IsSystemDarkMode();
        }
        break;
	case WM_TRAYICON:
	if (lParam == WM_RBUTTONUP) {
			// 右键点击托盘图标时的操作
			ShowContextMenu(hWnd);
		}
	break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_INPUT:
    {
        UINT dwSize=0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

        LPBYTE lpb = new BYTE[dwSize];
        if (lpb == NULL)
        {
            return 0;
        }

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
        {
            delete[] lpb;
            return 0;
        }

        RAWINPUT* raw = (RAWINPUT*)lpb;
        if (raw->header.dwType == RIM_TYPEKEYBOARD)
        {
            if (raw->data.keyboard.Message == WM_KEYDOWN || raw->data.keyboard.Message == WM_SYSKEYDOWN)
            {
                // 检测Caps Lock/Num Lock/Scroll Lock键
                if (raw->data.keyboard.VKey == VK_CAPITAL ||
                    raw->data.keyboard.VKey == VK_NUMLOCK ||
                    raw->data.keyboard.VKey == VK_SCROLL)
                {
                    PostMessage(hWnd, WM_UPDATEICONS, 0, 0);
                }
            }
        }

        delete[] lpb;
        break;
    }
    case WM_DESTROY:
    {
        // 卸载键盘钩子
        if (g_hKeyboardHook)
        {
            UnhookWindowsHookEx(g_hKeyboardHook);
            g_hKeyboardHook = NULL;
        }
        // 删除所有图标
        NOTIFYICONDATA nid = { 0 };
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;

        nid.uID = IDI_TRAY;
        Shell_NotifyIcon(NIM_DELETE, &nid);

        nid.uID = IDI_CAPSLOCK;
        Shell_NotifyIcon(NIM_DELETE, &nid);

        nid.uID = IDI_NUMLOCK;
        Shell_NotifyIcon(NIM_DELETE, &nid);

        nid.uID = IDI_SCROLLLOCK;
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
    }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

