#include "base.h"
#include "Soma.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst{};
TCHAR szTitle[MAX_LOADSTRING]{};
TCHAR szWindowClass[MAX_LOADSTRING]{};
HWND g_hmain{nullptr};


ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK LeftProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK RightProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BottomProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MSG msg{};

    InitCommonControls();

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex{};
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = WBRUSH;
    wcex.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    wcex.lpszClassName = MAIN_WINDOW_CLASS;
    wcex.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));

    if (!RegisterClassEx(&wcex)) return 0;

    // Left window
    wcex.lpfnWndProc   = LeftProc;
    wcex.hbrBackground = WBRUSH;
    wcex.lpszClassName = LEFT_WINDOW_CLASS;
    if (!RegisterClassEx(&wcex)) return 0;

    // Right window
    wcex.lpfnWndProc   = RightProc;
    wcex.hbrBackground  = WBRUSH;
    wcex.lpszClassName = RIGHT_WINDOW_CLASS;
    if (!RegisterClassEx(&wcex)) return 0;

    // Bottom window
    wcex.lpfnWndProc   = BottomProc;
    wcex.hbrBackground = WHITE;
    wcex.lpszClassName = BOTTOM_WINDOW_CLASS;
    if (!RegisterClassEx(&wcex)) return 0;

    return 1;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    g_hmain = CreateWindow(MAIN_WINDOW_CLASS,
                           SNOM,
                           WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, 0,
                           CW_USEDEFAULT, 0,
                           nullptr,
                           nullptr,
                           hInstance,
                           nullptr);

    if (!g_hmain) {
        return FALSE;
    }

    ShowWindow(g_hmain, nCmdShow);
    UpdateWindow(g_hmain);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return cfp.Callu(hWnd, message, wParam, lParam);
}
