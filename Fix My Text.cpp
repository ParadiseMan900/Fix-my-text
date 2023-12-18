//Code for "Fix my text" by Teslev Dmitry Sergeevich 2023
#include <windows.h>
#include <locale.h>
#include <stdio.h>
#include "resource.h"

#define GREATER_WIN10  22000

#define _HELP		0
#define _HIDESHOW	1
#define _EXIT		2

#define _SWAP		1
#define _UPDOWN		2

#define GREEN		10
#define BLUE		11
#define RED			12
#define YELLOW		14
#define WHITE		15


HWND console;
bool isExit = false;
bool isConsole = false;

struct LanguageLib
{
	const wchar_t* engChars;
	const wchar_t* otherChars;
	const wchar_t* sameChars;
	int sizeAll;
	int sizeSame;
};
struct StrData
{
	wchar_t* str = NULL;
	size_t size = 0;
	unsigned int format = CF_UNICODETEXT;
};
/////////////////////////////////////////////////////////////////////////////////////
const char* LoadAndWriteTXT(int nameID)
{
	const char* data = "";
	HINSTANCE handle = GetModuleHandleW(NULL);

	//Ссылка на ресурс
	HRSRC rcNameID = FindResourceW(handle, MAKEINTRESOURCEW(nameID),
		MAKEINTRESOURCEW(TEXTFILE));
	if (rcNameID)
	{
		//Данные в TXT (Все)
		HANDLE rcData = LoadResource(handle, rcNameID);
		if (rcData)
			data = (const char*)LockResource(rcData);
	}
	return data;
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
StrData GetBufer(bool print)
{
	StrData data{};
	if (OpenClipboard(NULL))
	{
		HANDLE hData;
		if (IsClipboardFormatAvailable(CF_UNICODETEXT))
		{
			hData = GetClipboardData(CF_UNICODETEXT);
			data.format = CF_UNICODETEXT;
		}
		else if (IsClipboardFormatAvailable(CF_DIB))
		{
			hData = GetClipboardData(CF_DIB);
			data.format = CF_DIB;
		}
		else return data;

		wchar_t* bufer = (wchar_t*)GlobalLock(hData);
		GlobalUnlock(hData);
		CloseClipboard();
		data.size = GlobalSize(hData);
		if (bufer != NULL)
		{
			data.str = (wchar_t*)malloc(data.size);
			if (data.str != NULL)
				memcpy(data.str, bufer, data.size);
		}
	}
	if (print)
	{
		SetColor(RED);
		printf("GetBufer:\t");
		SetColor(WHITE);
		if(data.format == CF_UNICODETEXT)
			wprintf(L"\"%s\"\n", data.str);
		else
		{
			SetColor(BLUE);
			printf("\"* PICTURE / КАРТИНКА *\"\n");
			SetColor(WHITE);
		}
	}
	return data;
}
void SetBufer(StrData data)
{
	HANDLE globAll = NULL;
	if (data.str != NULL)
	{
		globAll = GlobalAlloc(GMEM_DDESHARE, data.size);
		if (globAll)
		{
			void* globLock = GlobalLock(globAll);
			if(globLock)
				memcpy(globLock, data.str, data.size);
			GlobalUnlock(globAll);
		}

		if (OpenClipboard(NULL))
		{
			EmptyClipboard();
			SetClipboardData(data.format, globAll);
			CloseClipboard();
		}
	}
	SetColor(YELLOW);
	printf("SetBufer:\t");
	SetColor(WHITE);
	if (data.format == CF_UNICODETEXT)
		wprintf(L"\"%s\"\n", data.str);
	else
	{
		SetColor(BLUE);
		printf("\"* PICTURE / КАРТИНКА *\"\n");
		SetColor(WHITE);
	}
}
void LogMsg(const char* msg)
{
	SetColor(GREEN);
	printf("Message:\t%s\n", msg);
	SetColor(WHITE);
}
/////////////////////////////////////////////////////////////////////////////////////
wchar_t LUp(wchar_t letter)
{
	if ((letter >= L'a' && letter <= L'z')
		|| (letter >= L'а' && letter <= L'я'))
	{
		letter += (L'A' - L'a');
	}
	if (letter == L'ё')
		letter = L'Ё';
	return letter;
}
wchar_t LDown(wchar_t letter)
{
	if ((letter >= L'A' && letter <= L'Z')
		|| (letter >= L'А' && letter <= L'Я'))
	{
		letter += (L'a' - L'A');
	}
	if (letter == L'Ё')
		letter = L'ё';
	return letter;
}
bool LIsItEngWord(wchar_t* str, LanguageLib lib)
{
	int eng = 0;
	int other = 0;

	wchar_t letter;
	bool needToSkip = false;
	for (int i = 0; i < lstrlen(str); i++)
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
wchar_t LSwap(wchar_t ch, bool eng, LanguageLib lib)
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
void LChanger(wchar_t* str)
{
	LanguageLib RusLib{ L"`~@#$^&qwertyuiop[{]}|asdfghjkl;:'\"zxcvbnm,<.>/?" ,
		L"ёЁ\"№;:?йцукенгшщзхХъЪ/фывапролджЖэЭячсмитьбБюЮ.,",
			L"1234567890!%*()-_=+\\", 49 , 21 };

	bool isBig = false;
	bool isEng = false;

	size_t origLen = lstrlen(str) + 1;
	wchar_t* oneWord = NULL;
	wchar_t* result = (wchar_t*)malloc(origLen * sizeof(wchar_t));
	if (result == NULL)
		return;

	size_t counter = 0;
	size_t wordLen = 0;
	size_t lastID = 0;
	for (int i = 0; i < origLen; i++)
		if (str[i] == L' ' || str[i] == L'\t' || str[i] == L'\n' || str[i] == L'\0')
		{
			wordLen = i - lastID;
			oneWord = (wchar_t*)malloc((wordLen + 1) * sizeof(wchar_t));
			if (oneWord == NULL)
				continue;
			oneWord[wordLen] = L'\0';
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
	for (int i = 0; i < origLen - 1; i++)
		str[i] = result[i];
	free(oneWord);
	free(result);
}
void LSizer(wchar_t* str)
{
	for (int i = 0; i < lstrlen(str); i++)
		if (str[i] == LUp(str[i]))
			str[i] = LDown(str[i]);
		else str[i] = LUp(str[i]);
}
void LogicSwitch(StrData selected, int switcher)
{
	if (selected.str != NULL)
		switch (switcher)
		{
		case _SWAP:
			LChanger(selected.str);
			LogMsg("Смена языка выполнена");
			break;
		case _UPDOWN:
			LSizer(selected.str);
			LogMsg("Смена регистра выполнена");
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
int IsKeysPressed(keysComb comb)
{
	for (int i = 0; i < comb.len; i++)
	{
		if (GetAsyncKeyState(comb.names[i]) >= 0)
			return 0;
	}
	return comb.ID;
}
bool IsAllKeysUnpressed()
{
	//Проверка всех VK
	for (int i = VK_LBUTTON; i < VK_OEM_CLEAR; i++)
		if (GetAsyncKeyState(i) < 0)
			return false;
	return true;
}
void EmulateACombinationWithCtrl(wchar_t key)
{
	INPUT input[2] = { 0 };
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
NOTIFYICONDATA InterfaceConstructor()
{
	////	Узнаём версию Windows, т.к. на 11 винде не работает GetConsoleWindow()
	int osNUM = GREATER_WIN10;
	OSVERSIONINFOEXW osInfo{ 0 };
	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW) = nullptr;
	HINSTANCE gmh = GetModuleHandle(L"ntdll");
	if (gmh)
	{
		*(FARPROC*)&RtlGetVersion = GetProcAddress(gmh, "RtlGetVersion");
		if (RtlGetVersion)
		{
			osInfo.dwOSVersionInfoSize = sizeof(osInfo);
			RtlGetVersion(&osInfo);
			osNUM = (int)osInfo.dwBuildNumber;
		}
	}
	if (osNUM >= GREATER_WIN10)
		console = GetForegroundWindow();
	else console = GetConsoleWindow();
	ShowWindow(console, SW_HIDE);

	//Я Русский
	setlocale(LC_ALL, "");

	////	Регистрация класса (штука, чтобы "CALLBACK IconReaction" вызывалась)
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = IconReaction;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"TrayWin";
	RegisterClass(&wc);

	////	Создание отдельного окна для трея. Окно скрываем
	HWND trayWin = CreateWindowW(wc.lpszClassName, wc.lpszClassName, 0, 0, 0, 0, 0, 0, 0,
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
	return icon;
}
int main()
{
	////	Проверки, создание и регестрация окна, включение русского и т.п.
	//Это нужно, чтобы не хранились ненужные переменные
	NOTIFYICONDATA icon = InterfaceConstructor();
	Shell_NotifyIcon(NIM_ADD, &icon);
	
	////	Список ключей клавиатуры:
	//https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

	////	ВСЕ ПЕРЕМЕННЫЕ 
	int toSWAP_M[]{ VK_LWIN, VK_LCONTROL };
	int toUPDOWN_M[]{ VK_LWIN, VK_LMENU };
	keysComb toSWAP{ sizeof(toSWAP_M) / sizeof(toSWAP_M[0]), toSWAP_M, _SWAP };
	keysComb toUPDOWN{ sizeof(toUPDOWN_M) / sizeof(toUPDOWN_M[0]), toUPDOWN_M, _UPDOWN };

	StrData conservation{};
	StrData selected{};
	int switcher = 0;

	//Красивая консоль
	printf("%s\n", LoadAndWriteTXT(IDT_TEXT2));

	////	ЦАРЬ БАТЮШКА ЦИКЛ
	while (!isExit)
	{
		if (IsKeysPressed(toSWAP) || IsKeysPressed(toUPDOWN))
		{
			switcher = IsKeysPressed(toSWAP);
			if (!switcher)
				switcher = IsKeysPressed(toUPDOWN);

			printf("-----------------------------------------------\n");
					
			////	GET CONSERVATION;
			conservation = GetBufer(true);
			ClearBufer();

			////	LOGIC (CTRL+C / GET / *DO STAF* / SET / CTRL+V);
			selected.str = NULL;

			//Проблемные кнопки, мешающие копированию
			while (GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0
				|| GetAsyncKeyState(VK_MENU) < 0)
			{
				CallbackMessage();
				Sleep(10);
			}

			//Проверка на то что это не консоль (в консоли ctrl+c = смерть)
			if (GetForegroundWindow() == console)
			{
				LogMsg("Предотвращение ошибки. Не используйте FMT внутри консоли!");
				SetBufer(conservation);
				continue;
			}

			//Эмуляция Ctrl+C
			for (int i = 0; i < 10; i++)
			{
				EmulateACombinationWithCtrl('C');
				selected = GetBufer(false);
				if (selected.str != NULL)
					break;
				CallbackMessage();
			}
			//Скипаем логику если ничего не выделенно
			if (GetBufer(true).str == NULL)
			{
				LogMsg("Ничего не выделенно");
				SetBufer(conservation);
				continue;
			}
			if (selected.format == CF_DIB)
			{
				LogMsg("Пропуск картинки");
				SetBufer(conservation);
				continue;
			}
			ClearBufer();

			//Самая главная функция
			LogicSwitch(selected, switcher);                                        //<<<===
			SetBufer(selected);

			//Эмуляция Ctrl+V
			EmulateACombinationWithCtrl('V');

			////	SET CONSERVATION;
			SetBufer(conservation);
		}
		CallbackMessage();
		Sleep(10);
	}
	//Dectructor
	Shell_NotifyIcon(NIM_DELETE, &icon);

	free(conservation.str);
	free(selected.str);
	return 0;
}