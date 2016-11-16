#pragma once
#include <Windows.h>


class ItemMover {
public:
	ItemMover(class Window*);


	bool RetrieveItemCount();
	void RetrieveItemPositions();

	void MoveItems();


	~ItemMover();

private:
	void ReadRegistryString(HKEY, char*, char*, char*);

private:
	class Window* window_;

	HWND progMan_;
	HWND hwnd_;
	HANDLE explorer_;
	DWORD processID_;
	unsigned short itemCount_;
	int iconSize_;
	unsigned short screenWidth_;
	unsigned short screenHeight_;
	POINT* items_;
};