//Code for "Fix my text" by Teslev Dmitry Sergeevich
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <conio.h>
#include "resource.h"

#define pm_HELP 0
#define pm_HIDESHOW 1
#define pm_EXIT 2

#define pm_SWAP 1
#define pm_UPDOWN 2

HWND console;
int switcher = 0;
bool isExit = false;
bool isConsole = false;

LRESULT CALLBACK IconReaction(HWND window, UINT message, WPARAM commandID, LPARAM action)
{
	//Создание менюшки с кнопками
	if (message == WM_USER && action == WM_RBUTTONUP)
	{
		HMENU hMenu = CreatePopupMenu();
		UINT checker;
		AppendMenuA(hMenu, NULL, pm_HELP, "Помощь");

		if (isConsole)
			checker = MF_CHECKED;
		else checker = MF_UNCHECKED;
		AppendMenuA(hMenu, checker, pm_HIDESHOW, "Режим разработчика");

		AppendMenuA(hMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenuA(hMenu, NULL, pm_EXIT, "Выход");

		POINT pt;
		GetCursorPos(&pt);
		SetForegroundWindow(window);
		//TPM_BOTTOMALIGN == Меню правее курсора
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, window, 0);
	}

	//Реакция на кнопки в меню
	if (message == WM_COMMAND)
	{
		if (commandID == pm_HELP)
		{
			FILE* file = fopen("README.txt", "r");			// *.txt (ansi) файл
			if (file != NULL)
			{
				int counter = 0;
				while (!feof(file))
				{
					fgetc(file);
					counter++;
				}
				fseek(file, 0, 0);
				char* helpText = (char*)malloc(counter);
				helpText[counter - 1] = '\0';
				for (int i = 0; i < counter - 1; i++)
					helpText[i] = fgetc(file);

				MessageBoxA(NULL, helpText, "Помощь", NULL);
				fclose(file);
				free(helpText);
			}
			else MessageBoxA(NULL, "Не удалось найти файл с информацией", "ОШИБКА", MB_ICONERROR);
		}
		if (commandID == pm_HIDESHOW)
		{
			isConsole = !isConsole;
			if(isConsole)
				ShowWindow(console, SW_SHOW);
			else ShowWindow(console, SW_HIDE);
		}
			
		if (commandID == pm_EXIT)
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
	wchar_t* bufer = NULL;
	char* data = NULL;
	if (OpenClipboard(NULL))
	{
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		bufer = (wchar_t*)GlobalLock(hData);
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
		printf("GetBufer =  %s\n", data);
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
	printf("SetBufer =  %s\n", data);
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
bool LIsRus(char* str)
{
	int rus = 0;
	int eng = 0;
	char let = 0;
	for (int i = 0; i < strlen(str); i++)
	{
		let = LDown(str[i]);
		if ((let >= '0' && let <= '9') || let == '!' || let == '%' || let == '*' || let == '('
			|| let == ')' || let == '\\' || let == '-' || let == '_'
			|| let == '=' || let == '+')
			continue;
		if ((let >= 'а' && let <= 'я') || let == 'ё')
			rus++;
		else eng++;
	}
	if (rus > eng)
		return true;
	return false;
}
char LSwap(char ch, bool rus)
{
	if (rus)
	{
		//Rus to Eng
		if (ch == 'й')
			return 'q';
		if (ch == 'ц')
			return 'w';
		if (ch == 'у')
			return 'e';
		if (ch == 'к')
			return 'r';
		if (ch == 'е')
			return 't';
		if (ch == 'н')
			return 'y';
		if (ch == 'г')
			return 'u';
		if (ch == 'ш')
			return 'i';
		if (ch == 'щ')
			return 'o';
		if (ch == 'з')
			return 'p';
		if (ch == 'х')
			return '[';
		if (ch == 'ъ')
			return ']';
		if (ch == '/')
			return '|';
		if (ch == 'ф')
			return 'a';
		if (ch == 'ы')
			return 's';
		if (ch == 'в')
			return 'd';
		if (ch == 'а')
			return 'f';
		if (ch == 'п')
			return 'g';
		if (ch == 'р')
			return 'h';
		if (ch == 'о')
			return 'j';
		if (ch == 'л')
			return 'k';
		if (ch == 'д')
			return 'l';
		if (ch == 'ж')
			return ';';
		if (ch == 'э')
			return '\'';
		if (ch == 'я')
			return 'z';
		if (ch == 'ч')
			return 'x';
		if (ch == 'с')
			return 'c';
		if (ch == 'м')
			return 'v';
		if (ch == 'и')
			return 'b';
		if (ch == 'т')
			return 'n';
		if (ch == 'ь')
			return 'm';
		if (ch == 'б')
			return ',';
		if (ch == 'ю')
			return '.';
		if (ch == '.')
			return '/';
		if (ch == ',')
			return '?';
		if (ch == 'ё')
			return '`';
		if (ch == '"')
			return '@';
		if (ch == '№')
			return '#';
		if (ch == ';')
			return '$';
		if (ch == ':')
			return '^';
	}
	else
	{
		//Eng to Rus
		if (ch == 'q')
			return 'й';
		if (ch == 'w')
			return 'ц';
		if (ch == 'e')
			return 'у';
		if (ch == 'r')
			return 'к';
		if (ch == 't')
			return 'е';
		if (ch == 'y')
			return 'н';
		if (ch == 'u')
			return 'г';
		if (ch == 'i')
			return 'ш';
		if (ch == 'o')
			return 'щ';
		if (ch == 'p')
			return 'з';
		if (ch == '[')
			return 'х';
		if (ch == ']')
			return 'ъ';
		if (ch == '{')
			return 'Х';
		if (ch == '}')
			return 'Ъ';
		if (ch == '|')
			return '/';
		if (ch == 'a')
			return 'ф';
		if (ch == 's')
			return 'ы';
		if (ch == 'd')
			return 'в';
		if (ch == 'f')
			return 'а';
		if (ch == 'g')
			return 'п';
		if (ch == 'h')
			return 'р';
		if (ch == 'j')
			return 'о';
		if (ch == 'k')
			return 'л';
		if (ch == 'l')
			return 'д';
		if (ch == ';')
			return 'ж';
		if (ch == '\'')
			return 'э';
		if (ch == 'z')
			return 'я';
		if (ch == 'x')
			return 'ч';
		if (ch == 'c')
			return 'с';
		if (ch == 'v')
			return 'м';
		if (ch == 'b')
			return 'и';
		if (ch == 'n')
			return 'т';
		if (ch == 'm')
			return 'ь';
		if (ch == ',')
			return 'б';
		if (ch == '.')
			return 'ю';
		if (ch == '/')
			return '.';
		if (ch == '?')
			return ',';
		if (ch == '`')
			return 'ё';
		if (ch == '~')
			return 'Ё';
		if (ch == '@')
			return '"';
		if (ch == '#')
			return '№';
		if (ch == '$')
			return ';';
		if (ch == '^')
			return ':';
		if (ch == '&')
			return '?';
	}
	return ch;
}
char* LChanger(char* str)
{
	bool isRus = false;
	char* oneWord = (char*)malloc(sizeof(char));
	char* result = (char*)malloc(strlen(str) + 1);
	result[strlen(str)] = '\0';
	size_t counter = 0;
	size_t wordLen = 0;
	size_t last = 0;
	for (int i = 0; i < strlen(str) + 1; i++)
	{
		if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\0')
		{
			wordLen = i - last;
			oneWord = (char*)realloc(oneWord, sizeof(char) * (wordLen + 1));
			oneWord[wordLen] = '\0';
			for (int j = 0; j < wordLen; j++)
				oneWord[j] = str[last + j];

			////
			isRus = LIsRus(oneWord);
			for (int j = 0; j < wordLen; j++)
				if (oneWord[j] != LDown(oneWord[j]))
				{
					oneWord[j] = LDown(oneWord[j]);
					oneWord[j] = LSwap(oneWord[j], isRus);
					oneWord[j] = LUp(oneWord[j]);
				}
				else oneWord[j] = LSwap(oneWord[j], isRus);

			////
			for (int j = 0; j < wordLen; j++)
				result[last + j] = oneWord[j];
			result[i] = str[i];
			last = (size_t)i + 1;
		}
	}
	return result;
}

void LogicSwitch(char* selected)
{
	if (selected != NULL)
		switch (switcher)
		{
		case pm_SWAP:
			selected = LChanger(selected);
			break;
		case pm_UPDOWN:
			for (int i = 0; i < strlen(selected); i++)
				if (selected[i] == LUp(selected[i]))
					selected[i] = LDown(selected[i]);
				else selected[i] = LUp(selected[i]);
			break;
		default:
			return;
		}
	SetBufer(selected);
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
	console = GetConsoleWindow();
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
	keysComb toSWAP{ sizeof(toSWAP_M) / sizeof(toSWAP_M[0]), toSWAP_M, pm_SWAP };
	keysComb toUPDOWN{ sizeof(toUPDOWN_M) / sizeof(toUPDOWN_M[0]), toUPDOWN_M, pm_UPDOWN };

	char* conservation = NULL;
	char* bufer = NULL;
	char* selected = NULL;

	while (!isExit)
	{
		if (IsKeysPressed(toSWAP) || IsKeysPressed(toUPDOWN))
		{
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
			selected = GetBufer(true);
			ClearBufer();

			//Самая главноя функция
			LogicSwitch(selected);                                        //<<<===
			
			//Эмуляция Ctrl+V
			EmulateACombinationWithCtrl('V');

			////	SET CONSERVATION;
			SetBufer(conservation);
			printf("---------\n");
		}
		CallbackMessage();
		Sleep(10);
	}
	Shell_NotifyIcon(NIM_DELETE, &icon);
	free(bufer);
	free(conservation);
	free(selected);
}