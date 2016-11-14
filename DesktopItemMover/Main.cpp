#include <cstdio>

#include "Window.h"
#include "Application.h"


int WINAPI WinMain(_In_ HINSTANCE hInstance,
				   _In_ HINSTANCE hPrevInstance,
				   _In_ LPSTR lpCmdLine,
				   _In_ int nCmdShow) {
	
	Window window(hInstance, nCmdShow);

	Application application(&window);
	application.Start();

	return 0;
}