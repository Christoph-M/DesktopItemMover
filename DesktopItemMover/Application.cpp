#include "Application.h"
#include <CommCtrl.h>

#include "Window.h"


Application::Application(Window* window) :
	running_(true),
	message_(MSG{ 0 }),
	window_(window)
{ }


void Application::Start() {
	this->MessageLoop();
}

void Application::Stop() {
	PostMessage(NULL, WM_DESTROY, NULL, NULL);
}

void Application::MessageLoop() {
	HWND desktopHwnd = GetDesktopWindow();
	int num = ListView_GetItemCount(desktopHwnd);
	

	while (running_) {
		if (PeekMessage(&message_, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&message_);
			DispatchMessage(&message_);

			running_ = message_.message != WM_QUIT;
		}

		Sleep(1);
	}
}


Application::~Application() { }