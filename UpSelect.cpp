#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>

bool need_Сtrl;
bool need_Shift;
bool need_Alt;
bool need_Win;

int Сombination(int key, bool ctrl, bool shift, bool alt, bool win)
{
    need_Сtrl = !ctrl;
    need_Shift = !shift;
    need_Alt = !alt;
    need_Win = !win;
    return key;
}
bool keyPress(int key)
{
	return (GetAsyncKeyState(VK_CONTROL) || need_Сtrl)
		&& (GetAsyncKeyState(VK_MENU) || need_Alt)
		&& (GetAsyncKeyState(VK_LSHIFT) || need_Shift)
		&& (GetAsyncKeyState(VK_LWIN) || need_Win) && GetAsyncKeyState(key);
}
void Wait()
{
	while (InSendMessage()){}
}
char* GetBufer()
{
	OpenClipboard(NULL);
	char* data = (char*)GetClipboardData(CF_TEXT);
	EmptyClipboard();
	CloseClipboard();
	return data;
}
void SetBufer(char* data)
{
	size_t size = strlen(data) + 1;
	HGLOBAL dataMem = GlobalAlloc(GMEM_MOVEABLE, size);
	memcpy(GlobalLock(dataMem), data, size);
	GlobalUnlock(dataMem);

	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, dataMem);
	CloseClipboard();
}
void Logic()
{
	char* highlighting = GetBufer();
	if (highlighting == NULL)
		return;
	for (int i = 0; i < strlen(highlighting); i++)
		highlighting[i] = toupper(highlighting[i]);
	SetBufer(highlighting);
}
void CCP()
{
	HWND editWindow = FindWindowEx(GetForegroundWindow(), NULL, L"edit", NULL);
	if (editWindow == NULL)
		return;
	SendMessage(editWindow, WM_COPY, 0, 0);
	Wait();
	SendMessage(editWindow, WM_CLEAR, 0, 0);
	Wait();
	Logic();
	SendMessage(editWindow, WM_PASTE, 0, 0);
	Wait();
}
int main()
{
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false);
	while (true)
	{
		int key = Сombination(VK_ESCAPE, false, false, false, true);
		if (keyPress(key))
		{
			char* conservation = GetBufer();
			//printf("Сonservation =  %s\n", Сonservation);
			CCP();
			SetBufer(conservation);
			while(keyPress(key)){}
		}
	}
}

