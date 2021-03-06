#include "Window.h"


Window::Window() :
	wndClassEx_(WNDCLASSEX{ 0 }),
	hwnd_(NULL)
{
	WNDCLASSEX wndClass = { 0 };
		wndClass.cbSize = sizeof(wndClass);
		wndClass.style = CS_DROPSHADOW;
		wndClass.lpfnWndProc = WindowProcedure;
		wndClass.cbClsExtra = NULL;
		wndClass.hInstance = GetModuleHandle(NULL);
		wndClass.hIcon = NULL;
		wndClass.hCursor = NULL;
		wndClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = "DesktopItemMover";
		wndClass.hIconSm = NULL;

	RegisterClassEx(&wndClass);

	hwnd_ = CreateWindowEx(WS_EX_CLIENTEDGE, wndClass.lpszClassName, "Desktop Item mover", WS_SYSMENU | WS_SIZEBOX, 200, 200, 200, 200, NULL, NULL, NULL, NULL);
	ShowWindow(hwnd_, SW_SHOW);
}

Window::~Window() { }


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}