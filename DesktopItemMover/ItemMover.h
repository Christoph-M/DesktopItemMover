#pragma once
#include <Windows.h>
#include <vector>


class ItemMover {
public:
	ItemMover(class Window*);


	bool RetrieveItemCount();
	void RetrieveItemPositions();

	void MoveItems();


	~ItemMover();

private:
	struct Item {
		int index;
		POINT pos;
		Item* itemLeft;
		Item* itemRight;
		Item* itemTop;
		Item* itemBottom;
	};

private:
	void GetNeighbours();
	void SetItem(Item*);
	void SetPosOnScreen(Item*);
	void CheckNeighbours(Item*);

	void ReadRegistryString(HKEY, char*, char*, char*);

private:
	class Window* window_;

	HWND progMan_;
	HWND hwnd_;
	HANDLE explorer_;
	DWORD processID_;
	unsigned short itemCount_;
	int iconSize_;
	POINT itemSize_;
	unsigned short screenWidth_;
	unsigned short screenHeight_;
	std::vector<Item> items_;
};