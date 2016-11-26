#include "ItemMover.h"
#include <CommCtrl.h>
#include <cmath>

#include "Window.h"


ItemMover::ItemMover(class Window* window) :
	window_(window),
	progMan_(FindWindow("Progman", NULL)),
	processID_(0),
	itemCount_(0),
	screenWidth_(GetSystemMetrics(SM_CXFULLSCREEN)),
	screenHeight_(GetSystemMetrics(SM_CYFULLSCREEN)),
	items_(nullptr)
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

	return newCount == oldCount;
}

void ItemMover::RetrieveItemPositions() {
	if (items_) {
		delete[](items_);
		items_ = nullptr;
	}
	items_ = new POINT[itemCount_]{ -1 };

	POINT* pCoords = (POINT*)VirtualAllocEx(explorer_, NULL, sizeof(POINT), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(explorer_, pCoords, &pCoords, sizeof(POINT), NULL);

	POINT pt;
	for (int i = 0; i < itemCount_; ++i) {
		SendMessage(hwnd_, LVM_GETITEMPOSITION, (WPARAM)i, (LPARAM)pCoords);
		ReadProcessMemory(explorer_, pCoords, &pt, sizeof(POINT), NULL);
		items_[i] = pt;
	}

	VirtualFreeEx(explorer_, pCoords, 0, MEM_RELEASE);
}

void ItemMover::MoveItems() {
	for (int i = 0; i < itemCount_; ++i) {
		if (items_[i].x >= 0 && items_[i].y >= 0) {
			POINT itemTopLeft = { items_[i].x, items_[i].y };
			POINT itemBottomRight = { itemTopLeft.x + iconSize_, itemTopLeft.y + iconSize_ };

			if (hwnd_ != WindowFromPoint(itemTopLeft) && hwnd_ != WindowFromPoint(itemBottomRight)) {
				RECT windowDim;
				GetWindowRect(window_->Hwnd(), &windowDim);
				POINT windowCenter = { (windowDim.right - windowDim.left) / 2 + windowDim.left, (windowDim.bottom - windowDim.top) / 2 + windowDim.top };
				POINT itemCenter = { itemTopLeft.x + iconSize_ / 2, itemTopLeft.y + iconSize_ / 2 };
				POINT dir = { itemCenter.x - windowCenter.x, itemCenter.y - windowCenter.y };

				if (abs(dir.x) > abs(dir.y)) {
					items_[i].x += (iconSize_ * 2) * dir.x / abs(dir.x);
				} else {
					items_[i].y += (iconSize_ * 2) * dir.y / abs(dir.y);
				}

				if (items_[i].x < 40) {
					items_[i].x += (iconSize_ * 2);
				} else if (items_[i].x + iconSize_ > screenWidth_ - 64) {
					items_[i].x -= (iconSize_ * 2);
				}

				if (items_[i].y < 40) {
					items_[i].y += (iconSize_ * 2);
				} else if (items_[i].y + iconSize_ > screenHeight_ - 64) {
					items_[i].y -= (iconSize_ * 2);
				}


				SendMessage(hwnd_, LVM_SETITEMPOSITION, (WPARAM)i, MAKELPARAM((int)items_[i].x, (int)items_[i].y));
			}
		}
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


ItemMover::~ItemMover() {
	delete[](items_);
}