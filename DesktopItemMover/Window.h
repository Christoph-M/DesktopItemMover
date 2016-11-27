#pragma once
#include <Windows.h>


LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

class Window {
public:
	Window();


	HWND Hwnd() { return hwnd_; }


	~Window();

private:
	WNDCLASSEX wndClassEx_;
	HWND hwnd_;
};