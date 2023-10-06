//Code for "Fix my text" by Teslev Dmitry Sergeevich
#include <windows.h>
#include <iostream>
#include "resource.h"

#define _HELP 0
#define _HIDESHOW 1
#define _EXIT 2

#define _SWAP 1
#define _UPDOWN 2

#define RED 12
#define YELLOW 14
#define WHITE 15

HWND console;
int switcher = 0;
bool isExit = false;
bool isConsole = false;

struct LanguageLib
{
	const char* engChars;
	const char* otherChars;
	const char* sameChars;
	int sizeAll;
	int sizeSame;
};
/////////////////////////////////////////////////////////////////////////////////////
const char* LoadAndWriteTXT(int nameID)
{
	HINSTANCE handle = GetModuleHandle(NULL);
	//Ссылка на ресурс
	HRSRC rcNameID = FindResource(handle, MAKEINTRESOURCE(nameID), MAKEINTRESOURCE(TEXTFILE));
	//Данные в TXT (Все)
	HGLOBAL rcData = LoadResource(handle, rcNameID);
	return (const char*)LockResource(rcData);
}
LRESULT CALLBACK IconReaction(HWND window, UINT message, WPARAM commandID, LPARAM action)
{
	//Создание менюшки с кнопками
	if ((message == WM_USER) && (action == WM_RBUTTONUP || action == WM_LBUTTONUP || action == WM_MBUTTONDBLCLK))
	{
		HMENU hMenu = CreatePopupMenu();
		if (action == WM_MBUTTONDBLCLK)
		{
			UINT checker = NULL;
			if (isConsole)
				checker = MF_CHECKED;
			else checker = MF_UNCHECKED;
			AppendMenu(hMenu, checker, _HIDESHOW, L"Режим разработчика");
			SetMenuItemBitmaps(hMenu, _HIDESHOW, MF_BYCOMMAND,
				LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP3)), 0);
		}
		else
		{
			AppendMenu(hMenu, NULL, _HELP, L"Помощь");
			AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);	
			AppendMenu(hMenu, NULL, _EXIT, L"Выход");

			SetMenuItemBitmaps(hMenu, _HELP, MF_BYCOMMAND,
				LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1)), 0);
			SetMenuItemBitmaps(hMenu, _EXIT, MF_BYCOMMAND,
				LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP2)), 0);
		}
		POINT pt;
		GetCursorPos(&pt);
		SetForegroundWindow(window);
		//TPM_BOTTOMALIGN == Меню правее курсора
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, window, 0);
	}

	//Реакция на кнопки в меню
	if (message == WM_COMMAND)
	{
		if (commandID == _HELP)
			MessageBoxA(NULL, LoadAndWriteTXT(IDT_TEXT1), "Помощь", NULL);

		if (commandID == _HIDESHOW)
		{
			isConsole = !isConsole;
			if (isConsole)
				ShowWindow(console, SW_SHOW);
			else ShowWindow(console, SW_HIDE);
		}

		if (commandID == _EXIT)
			isExit = true;
	}
	return DefWindowProc(window, message, commandID, action);
}
void CallbackMessage()
{
	MSG msg = { 0 };
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void SetColor(int colorID)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorID);
}
void ClearBufer()
{
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();
		CloseClipboard();
	}
}
char* GetBufer(bool print)
{
	char* data = NULL;
	if (OpenClipboard(NULL))
	{
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		wchar_t* bufer = (wchar_t*)GlobalLock(hData);
		GlobalUnlock(hData);
		CloseClipboard();
		if (bufer != NULL)
		{
			int buferSize = WideCharToMultiByte(CP_ACP, 0, bufer, -1, 0, 0, 0, 0);
			data = new char[buferSize];
			WideCharToMultiByte(CP_ACP, 0, bufer, -1, data, buferSize, 0, 0);
		}
	}
	if (print)
	{
		SetColor(RED);
		printf("SetBufer: ");
		SetColor(WHITE);
		printf("%s\n", data);
	}
	return data;
}
void SetBufer(char* data)
{
	HGLOBAL globAll = NULL;
	if (data != NULL)
	{
		int dataSize = MultiByteToWideChar(CP_ACP, 0, data, -1, 0, 0);
		wchar_t* bufer = new wchar_t[dataSize];
		MultiByteToWideChar(CP_ACP, 0, data, -1, bufer, dataSize);

		size_t buferLen = (wcslen(bufer) + 1) * sizeof(wchar_t);
		globAll = GlobalAlloc(GMEM_DDESHARE, buferLen);
		memcpy(GlobalLock(globAll), bufer, buferLen);
		GlobalUnlock(globAll);
		if (OpenClipboard(NULL))
		{
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, globAll);
			CloseClipboard();
		}
	}
	SetColor(YELLOW);
	printf("SetBufer: ");
	SetColor(WHITE);
	printf("%s\n", data);
}
/////////////////////////////////////////////////////////////////////////////////////
char LUp(char letter)
{
	if ((letter >= 'a' && letter <= 'z')
		|| (letter >= 'а' && letter <= 'я'))
	{
		letter = letter + ('A' - 'a');
	}
	if (letter == 'ё')
		letter = 'Ё';
	return letter;
}
char LDown(char letter)
{
	if ((letter >= 'A' && letter <= 'Z')
		|| (letter >= 'А' && letter <= 'Я'))
	{
		letter = letter + ('a' - 'A');
	}
	if (letter == 'Ё')
		letter = 'ё';
	return letter;
}
bool LIsItEngWord(char* str, LanguageLib lib)
{
	int eng = 0;
	int other = 0;

	char letter;
	bool needToSkip = false;
	for (int i = 0; i < strlen(str); i++)
	{
		needToSkip = false;
		letter = LDown(str[i]);

		for (int j = 0; j < lib.sizeSame; j++)
			if (letter == lib.sameChars[j])
			{
				needToSkip = true;
				break;
			}		
		if (needToSkip)
			continue;

		for (int j = 0; j < lib.sizeAll; j++)
			if (letter == lib.otherChars[j])
			{
				other++;
				needToSkip = true;
				break;
			}
		if (needToSkip)
			continue;
		eng++;
	}
	if (other > eng)
		return false;
	return true;
}
char LSwap(char ch, bool eng, LanguageLib lib)
{
	//Смена между языками
	if (eng)
	{
		for (int i = 0; i < lib.sizeAll; i++)
			if (ch == lib.engChars[i])
				return lib.otherChars[i];
	}
	else for (int i = 0; i < lib.sizeAll; i++)
			if (ch == lib.otherChars[i])
				return lib.engChars[i];
	return ch;
}
void LChanger(char* str)
{
	LanguageLib RusLib{ "`~@#$^&qwertyuiop[{]}|asdfghjkl;:'\"zxcvbnm,<.>/?" ,
"ёЁ\"№;:?йцукенгшщзхХъЪ/фывапролджЖэЭячсмитьбБюЮ.,",
 "1234567890!%*()-_=+\\", 49 , 21 };

	bool isBig = false;
	bool isEng = false;

	size_t origLen = strlen(str) + 1;
	char* oneWord = NULL;
	char* result = new char[origLen];

	size_t counter = 0;
	size_t wordLen = 0;
	size_t lastID = 0;
	for (int i = 0; i < origLen; i++)
		if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\0')
		{
			wordLen = i - lastID;
			oneWord = new char[wordLen + 1];
			oneWord[wordLen] = '\0';
			for (int j = 0; j < wordLen; j++)
				oneWord[j] = str[lastID + j];

			////
			isEng = LIsItEngWord(oneWord, RusLib);
			for (int j = 0; j < wordLen; j++)
				{
				isBig = false;
				if (oneWord[j] != LDown(oneWord[j]))
					isBig = true;
				oneWord[j] = LSwap(LDown(oneWord[j]), isEng, RusLib);
				if(isBig)
					oneWord[j] = LUp(oneWord[j]);
			};

			////
			for (int j = 0; j < wordLen; j++)
				result[lastID + j] = oneWord[j];
			result[i] = str[i];
			lastID = (size_t)i + 1;
		}
	for (int i = 0; i < origLen-1; i++)
		str[i] = result[i];
	delete[] oneWord;
	delete[] result;
}
void LSizer(char* str)
{
	for (int i = 0; i < strlen(str); i++)
		if (str[i] == LUp(str[i]))
			str[i] = LDown(str[i]);
		else str[i] = LUp(str[i]);
}
void LogicSwitch(char* selected)
{
	if (selected != NULL)
		switch (switcher)
		{
		case _SWAP:
			LChanger(selected);
			break;
		case _UPDOWN:
			LSizer(selected);
			break;
		default:
			return;
		}
}
/////////////////////////////////////////////////////////////////////////////////////
struct keysComb
{
	int len;
	int* names;
	int ID;
};
bool IsKeysPressed(keysComb comb)
{
	for (int i = 0; i < comb.len; i++)
	{
		if (GetAsyncKeyState(comb.names[i]) >= 0)
			return false;
	}
	if (comb.ID != 0)
		switcher = comb.ID;
	return true;
}
bool IsAllKeysUnpressed()
{
	//Проверка всех VK
	for (int i = VK_LBUTTON; i < VK_OEM_CLEAR; i++)
		if (GetAsyncKeyState(i) < 0)
			return false;
	return true;
}
void EmulateACombinationWithCtrl(char key)
{
	INPUT input[2] = { 0 };
	int keysMuss[] = { VK_CONTROL, key };
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = VK_CONTROL;
	input[0].ki.dwFlags = 0;
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = key;
	input[1].ki.dwFlags = 0;
	SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
	Sleep(10);
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;
	input[0].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
	Sleep(10);
}
/////////////////////////////////////////////////////////////////////////////////////

int main()
{
	////	Узнаём версию Windows, т.к. на 11 винде не работает GetConsoleWindow()
	int osNUM = 0;
	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW) = nullptr;
	OSVERSIONINFOEXW osInfo{ 0 };
	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandle(L"ntdll"), "RtlGetVersion");
	if (RtlGetVersion != NULL)
	{
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);
		osNUM = (int)osInfo.dwBuildNumber;
	}
	if (osNUM >= 22000)
		console = GetForegroundWindow();
	else console = GetConsoleWindow();
	ShowWindow(console, SW_HIDE);

	//Я Русский
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	////	Регистрация класса (штука, чтобы "CALLBACK IconReaction" вызывалась)
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = IconReaction;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"TrayWin";
	RegisterClass(&wc);

	////	Создание отдельного окна для трея. Окно скрываем
	HWND trayWin = CreateWindow(wc.lpszClassName, L"TrayWin", 0, 0, 0, 0, 0, 0, 0,
		wc.hInstance, 0);

	////	Добавление окна (в виде иконки) в трей
	NOTIFYICONDATA icon = { 0 };
	icon.cbSize = sizeof(icon);
	icon.hWnd = trayWin;
	icon.uVersion = NOTIFYICON_VERSION;
	icon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	icon.uCallbackMessage = WM_USER;
	//IDI_ICON1 - ID Иконки из файла ресурсов.
	icon.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	wcscpy_s(icon.szTip, L"\"Fix my text\"\nНажмите ПКМ для открытия меню");
	Shell_NotifyIcon(NIM_ADD, &icon);

	////	Список ключей клавиатуры:
	//https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

	////	ВСЕ ФУКЦИИ 
	int toSWAP_M[]{ VK_LWIN, VK_LCONTROL };
	int toUPDOWN_M[]{ VK_LWIN, VK_LMENU };
	keysComb toSWAP{ sizeof(toSWAP_M) / sizeof(toSWAP_M[0]), toSWAP_M, _SWAP };
	keysComb toUPDOWN{ sizeof(toUPDOWN_M) / sizeof(toUPDOWN_M[0]), toUPDOWN_M, _UPDOWN };

	char* conservation = NULL;
	char* bufer = NULL;
	char* selected = NULL;

	printf("%s\n", LoadAndWriteTXT(IDT_TEXT2));

	while (!isExit)
	{
		if (IsKeysPressed(toSWAP) || IsKeysPressed(toUPDOWN))
		{
			printf("-----------------------------------------------\n");
			////	GET CONSERVATION;
			conservation = GetBufer(true);
			ClearBufer();

			////	LOGIC (CTRL+C / GET / *DO STAF* / SET / CTRL+V);
			selected = NULL;

			//Проблемные кнопки, мешающие копированию
			while (GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0
				|| GetAsyncKeyState(VK_MENU) < 0)
			{
				CallbackMessage();
				Sleep(10);
			}

			//Эмуляция Ctrl+C
			for (int i = 0; i < 10; i++)
			{
				EmulateACombinationWithCtrl('C');
				selected = GetBufer(false);
				if (selected != NULL)
					break;
				CallbackMessage();
			}

			//Скипаем логику если ничего не выделенно
			if (GetBufer(true) == NULL)
			{
				SetBufer(conservation);
				continue;
			}
			ClearBufer();

			//Самая главноя функция
			LogicSwitch(selected);                                        //<<<===
			SetBufer(selected);

			//Эмуляция Ctrl+V
			EmulateACombinationWithCtrl('V');

			////	SET CONSERVATION;
			SetBufer(conservation);
		}
		CallbackMessage();
		Sleep(10);
	}
	Shell_NotifyIcon(NIM_DELETE, &icon);

	delete[] bufer;
	delete[] conservation;
	delete[] selected;
}