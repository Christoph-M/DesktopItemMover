#pragma once
#include <Windows.h>


class Application {
public:
	Application(class Window*);


	void Start();
	void Stop();


	~Application();

private:
	void MessageLoop();

private:
	bool running_;
	MSG message_;
	class Window* window_;
};