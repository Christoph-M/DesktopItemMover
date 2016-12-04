#include <cstdio>

#include "Window.h"
#include "Application.h"


int main() {
	Window window = Window();

	Application application = Application(&window);
	application.Start();

	return 0;
}
