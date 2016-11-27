#include "ItemMover.h"
#include <CommCtrl.h>
#include <cmath>
#include <cstdio>

#include "Window.h"


ItemMover::ItemMover(class Window* window) :
	window_(window),
	progMan_(FindWindow("Progman", NULL)),
	processID_(0),
	itemCount_(0),
	itemSize_{120, 105},
	screenWidth_(GetSystemMetrics(SM_CXFULLSCREEN)),
	screenHeight_(GetSystemMetrics(SM_CYFULLSCREEN))
{
	hwnd_ = FindWindowEx(progMan_, 0, "SHELLDLL_DefView", NULL);
	hwnd_ = FindWindowEx(hwnd_, 0, "SysListView32", NULL);

	GetWindowThreadProcessId(hwnd_, &processID_);
	explorer_ = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, processID_);

	this->RetrieveItemCount();
	this->RetrieveItemPositions();

	char szIconSize[1024] = "";
	this->ReadRegistryString(HKEY_CURRENT_USER, "Control Panel\\Desktop\\WindowMetrics", "Shell Icon Size", szIconSize);
	iconSize_ = atoi(szIconSize);

	SetWindowLong(hwnd_, GWL_EXSTYLE, GetWindowLong(hwnd_, GWL_EXSTYLE) | LVS_EX_DOUBLEBUFFER);
	long style = GetWindowLong(hwnd_, GWL_STYLE);
	if (style & LVS_AUTOARRANGE) {
		SetWindowLong(hwnd_, GWL_STYLE, style & ~LVS_AUTOARRANGE);
	}
}


bool ItemMover::RetrieveItemCount() {
	int oldCount = itemCount_;
	int newCount = ListView_GetItemCount(hwnd_);
	itemCount_ = newCount;

	if (newCount > oldCount) {
		for (int i = 0; i < newCount - oldCount; ++i) {
			Item newItem = { };
			newItem.index = -1;
			newItem.pos = { 0 };
			newItem.itemLeft = nullptr;
			newItem.itemRight = nullptr;
			newItem.itemTop = nullptr;
			newItem.itemBottom = nullptr;
			items_.push_back(newItem);
		}
	} else if (newCount < oldCount) {
		for (int i = 0; i < oldCount - newCount; ++i) items_.erase(items_.end());
	}

	return newCount == oldCount;
}

void ItemMover::RetrieveItemPositions() {
	POINT* pCoords = (POINT*)VirtualAllocEx(explorer_, NULL, sizeof(POINT), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(explorer_, pCoords, &pCoords, sizeof(POINT), NULL);

	POINT pt;
	for (int i = 0; i < itemCount_; ++i) {
		SendMessage(hwnd_, LVM_GETITEMPOSITION, (WPARAM)i, (LPARAM)pCoords);
		ReadProcessMemory(explorer_, pCoords, &pt, sizeof(POINT), NULL);
		items_[i].index = i;
		items_[i].pos = pt;
	}

	VirtualFreeEx(explorer_, pCoords, 0, MEM_RELEASE);
	
	this->GetNeighbours();
}

void ItemMover::GetNeighbours() {
	for (int i = 0; i < items_.size(); ++i) {
		items_[i].itemLeft = nullptr;
		items_[i].itemRight = nullptr;
		items_[i].itemTop = nullptr;
		items_[i].itemBottom = nullptr;

		for (int f = 0; f < items_.size(); ++f) {
			if (items_[f].pos.y == items_[i].pos.y && items_[f].pos.x == items_[i].pos.x - itemSize_.x) {
				items_[i].itemLeft = &items_[f];
			} else if (items_[f].pos.y == items_[i].pos.y && items_[f].pos.x == items_[i].pos.x + itemSize_.x) {
				items_[i].itemRight = &items_[f];
			} else if (items_[f].pos.x == items_[i].pos.x && items_[f].pos.y == items_[i].pos.y - itemSize_.y) {
				items_[i].itemTop = &items_[f];
			} else if (items_[f].pos.x == items_[i].pos.x && items_[f].pos.y == items_[i].pos.y + itemSize_.y) {
				items_[i].itemBottom = &items_[f];
			}
		}
	}
}

void ItemMover::MoveItems() {
	for (int i = 0; i < itemCount_; ++i) {
		if (items_[i].pos.x >= 0 && items_[i].pos.y >= 0) {
			POINT itemTopLeft = { items_[i].pos.x, items_[i].pos.y };
			POINT itemBottomRight = { items_[i].pos.x + itemSize_.x, items_[i].pos.y + itemSize_.y };

			if (hwnd_ != WindowFromPoint(itemTopLeft) && hwnd_ != WindowFromPoint(itemBottomRight)) {
				RECT windowDim;
				GetWindowRect(window_->Hwnd(), &windowDim);
				POINT windowCenter = { (windowDim.right - windowDim.left) / 2 + windowDim.left, (windowDim.bottom - windowDim.top) / 2 + windowDim.top };
				POINT itemCenter = { itemTopLeft.x + itemSize_.x / 2, itemTopLeft.y + itemSize_.y / 2 };
				POINT dir = { itemCenter.x - windowCenter.x, itemCenter.y - windowCenter.y };
				
				if (abs(dir.x) > abs(dir.y)) {
					items_[i].pos.x += itemSize_.x * dir.x / abs(dir.x);
				} else {
					items_[i].pos.y += itemSize_.y * dir.y / abs(dir.y);
				}

				this->SetItem(&items_[i]);
			}
		}
	}
}

void ItemMover::SetItem(Item* item) {
	this->SetPosOnScreen(item);
	this->CheckNeighbours(item);

	SendMessage(hwnd_, LVM_SETITEMPOSITION, (WPARAM)item->index, MAKELPARAM((int)item->pos.x, (int)item->pos.y));
}

void ItemMover::SetPosOnScreen(Item* item) {
	if (item->pos.x < 36) {
		item->pos.x += itemSize_.x;
		item->pos.y += itemSize_.y;
		if (item->pos.y >= screenHeight_ - itemSize_.y - 2) {
			item->pos.y -= itemSize_.y * 2;
		}
	} else if (item->pos.x >= screenWidth_ - itemSize_.x - 36) {
		item->pos.x -= itemSize_.x;
		item->pos.y -= itemSize_.y;
		if (item->pos.y < 2) {
			item->pos.y += itemSize_.y * 2;
		}
	}

	if (item->pos.y < 2) {
		item->pos.y += itemSize_.y;
		item->pos.x -= itemSize_.x;
		if (item->pos.x < 36) {
			item->pos.x += itemSize_.x * 2;
		}
	} else if (item->pos.y >= screenHeight_ - itemSize_.y - 2) {
		item->pos.y -= itemSize_.y;
		item->pos.x += itemSize_.x;
		if (item->pos.x >= screenWidth_ - itemSize_.x - 36) {
			item->pos.x -= itemSize_.x * 2;
		}
	}
}

void ItemMover::CheckNeighbours(Item* item) {
	if (item->itemLeft && item->pos.x == item->itemLeft->pos.x) {
		item->itemLeft->pos.x -= itemSize_.x;
		this->SetItem(item->itemLeft);
	} else if (item->itemRight && item->pos.x == item->itemRight->pos.x) {
		item->itemRight->pos.x += itemSize_.x;
		this->SetItem(item->itemRight);
	} else if (item->itemTop && item->pos.y == item->itemTop->pos.y) {
		item->itemTop->pos.y -= itemSize_.y;
		this->SetItem(item->itemTop);
	} else if (item->itemBottom && item->pos.y == item->itemBottom->pos.y) {
		item->itemBottom->pos.y += itemSize_.y;
		this->SetItem(item->itemBottom);
	}
}


void ItemMover::ReadRegistryString(HKEY hKeyMom, char* pKeyName, char* pValName, char* pValData) {
	HKEY hKey = 0;
	DWORD dw;

	if (RegOpenKeyEx(hKeyMom, pKeyName, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		strcpy_s(pValData, sizeof(pValData), "");
	}

	LONG rtn = RegQueryValueEx(hKey, pValName, NULL, &dw, NULL, NULL);
	if ((rtn != ERROR_SUCCESS) || (dw != REG_SZ)) {
		RegCloseKey(hKey);
		strcpy_s(pValData, sizeof(pValData), "");
	}

	dw = 1024;
	RegQueryValueEx(hKey, pValName, NULL, NULL, (BYTE*)pValData, &dw);
	RegCloseKey(hKey);
}


ItemMover::~ItemMover() { }