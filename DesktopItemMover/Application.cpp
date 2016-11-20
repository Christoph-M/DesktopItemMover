#include "Application.h"

#include <process.h>

#include "Window.h"
#include "ItemMover.h"


class Window* g_Window;

Application::Application(Window* window) :
	running_(true),
	message_(MSG{ 0 }),
	window_(window)
{
	g_Window = window_;
}


void Application::Start() {
	this->MessageLoop();
}

void Application::Stop() {
	PostMessage(NULL, WM_DESTROY, NULL, NULL);
}

void Application::MessageLoop() {
	HANDLE hThread;
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadLoop, &running_, 0, NULL);

	while (running_) {
		if (PeekMessage(&message_, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&message_);
			DispatchMessage(&message_);

			running_ = message_.message != WM_QUIT;
		}

		Sleep(1);
	}

	WaitForMultipleObjects(1, &hThread, true, INFINITE);
}

unsigned __stdcall ThreadLoop(void* params) {
	bool* running = (bool*)params;
	ItemMover itemMover = ItemMover(g_Window);

	while (*running) {
		itemMover.RetrieveItemCount();
		itemMover.RetrieveItemPositions();
		itemMover.MoveItems();

		Sleep(1);
	}

	_endthreadex(0);
	return 0;
}


Application::~Application() { }