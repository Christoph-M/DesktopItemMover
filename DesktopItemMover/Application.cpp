#include "Application.h"

#include "Window.h"
#include "ItemMover.h"


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
	ItemMover itemMover = ItemMover(window_);

	while (running_) {
		if (PeekMessage(&message_, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&message_);
			DispatchMessage(&message_);

			running_ = message_.message != WM_QUIT;
		}

		itemMover.RetrieveItemCount();
		itemMover.RetrieveItemPositions();
		itemMover.MoveItems();

		Sleep(1);
	}
}


Application::~Application() { }