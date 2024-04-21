//Code for "Fix my text" by Teslev Dmitry Sergeevich 2023 - 2024
#include <windows.h>
#include <locale.h>
#include <stdio.h>
#include "resource.h"
#include <wininet.h>
#pragma comment(lib, "WinInet.lib") 

//FMT Version data
#define	VERSION			"v6.0"
#define VERSIONID		10
//Icon Reaction IDs
#define IR_HELP			0
#define IR_HIDESHOW		1
#define IR_SYSSWITCH	2
#define IR_EXIT			3
#define IR_NEWVER		4
//Logic Switch IDs
#define LS_SWAP			1
#define LS_UPDOWN		2
//Set Color IDs
#define SC_GREEN		10
#define SC_BLUE			11
#define SC_RED			12
#define SC_YELLOW		14
#define SC_WHITE		15
#define SetColor(colID) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colID)
//Time Sleep IDs
#define TS_PROCESSOR	10
#define TS_NORMAL		15
#define TS_SLOW_APP		250
//Default struct data
#define D_STRDATA		{NULL, 0, CF_UNICODETEXT}
//Window Class Name
#define WCN_ICON		L"TrayWin"
#define WCN_HELP		L"HELP"
//Help Window Size
#define WH_WIDTH		400
#define WH_HEIGHT		250
//Help Timer Settings
#define TID_INTRO		1
#define ANIM_SIZE		45
#define ANIM_INTERVAL	18

HWND console =			NULL;
HWND help =				NULL;
BOOL isExit =			FALSE;
BOOL isVisible =		FALSE;
BOOL isChangeLang =		TRUE;
BOOL isNewVersion =		FALSE;

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
	wchar_t* str;
	size_t size;
	unsigned int format;
};
struct keysComb
{
	unsigned int len;
	int* names;
	int ID;
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
HWND FindMainWindow()
{
	DWORD crawlProcID = 0;
	DWORD mainProcID = GetCurrentProcessId();
	HWND crawlWin = GetTopWindow(GetDesktopWindow());
	while (crawlWin)
	{
		if (IsWindowVisible(crawlWin))
		{
			GetWindowThreadProcessId(crawlWin, &crawlProcID);
			if (crawlProcID == mainProcID)
				return crawlWin;
		}
		crawlWin = GetWindow(crawlWin, GW_HWNDNEXT); //Обход всех окон
	}
	return NULL;
}
void AddDebugConsole()
{
	AllocConsole();
	FILE* safeTemp;
	freopen_s(&safeTemp, "CON", "w", stdout);
	SetConsoleTitleW(L"Fix My Text - Debug mode");
	console = FindMainWindow();
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "");
}
void PrintCenterText(HDC hdc, SIZE size, unsigned int y, wchar_t* text,
	COLORREF colour, double fontScale, BOOL isBold, BOOL isItalic)
{
	HFONT font = CreateFontW((int)(30 * fontScale), (int)(15 * fontScale), 0, 0, isBold ? FW_EXTRABOLD : FW_NORMAL,
		isItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		VARIABLE_PITCH, L"Consolas");
	SelectObject(hdc, font);
	GetTextExtentPoint32W(hdc, text, lstrlenW(text), &size);
	if (colour != -1)
	{
		HPEN pen = CreatePen(PS_SOLID, 0, colour);
		HBRUSH brush = CreateSolidBrush(colour);
		SelectObject(hdc, pen);
		SelectObject(hdc, brush);
		RoundRect(hdc, (WH_WIDTH >> 1) - (size.cx >> 1) - 5, y - 2,
			(WH_WIDTH >> 1) + (size.cx >> 1) + 5, y + size.cy + 2, 15, 15);
		DeleteObject(pen);
		DeleteObject(brush);
	}
	TextOutW(hdc, (WH_WIDTH >> 1) - (size.cx >> 1), y, text, lstrlenW(text));
	DeleteObject(font);
}
LRESULT CALLBACK IconReaction(HWND window, UINT message, WPARAM commandID, LPARAM action)
{
	//Создание менюшки с кнопками
	if ((message == WM_USER) &&
		(action == WM_RBUTTONUP || action == WM_LBUTTONUP || action == WM_MBUTTONDBLCLK))
	{
		HMENU hMenu = CreatePopupMenu();
		if (action == WM_MBUTTONDBLCLK)
		{
			AppendMenuW(hMenu, isVisible ? MF_CHECKED : MF_UNCHECKED, IR_HIDESHOW,
				L"Режим разработчика");
			SetMenuItemBitmaps(hMenu, IR_HIDESHOW, MF_BYCOMMAND,
				LoadBitmapW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDB_BITMAP3)), 0);
		}
		else
		{
			if (isNewVersion)
			{
				AppendMenuW(hMenu, 0, IR_NEWVER, L"Доступна новая версия!");
				AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
			}
			AppendMenuW(hMenu, 0, IR_HELP, L"Помощь");
			AppendMenuW(hMenu, isChangeLang ? MF_CHECKED : MF_UNCHECKED, IR_SYSSWITCH,
				L"Автоматически менять язык системы");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuW(hMenu, 0, IR_EXIT, L"Выход");

			SetMenuItemBitmaps(hMenu, IR_HELP, MF_BYCOMMAND,
				LoadBitmapW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDB_BITMAP1)), 0);
			SetMenuItemBitmaps(hMenu, IR_SYSSWITCH, MF_BYCOMMAND,
				LoadBitmapW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDB_BITMAP4)), 0);
			SetMenuItemBitmaps(hMenu, IR_EXIT, MF_BYCOMMAND,
				LoadBitmapW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDB_BITMAP2)), 0);
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
		switch (commandID)
		{
		case IR_HELP:
			////	Создание окна.
			if (!help)
				help = CreateWindowW(WCN_HELP, L"Помощь", WS_SYSMENU | WS_VISIBLE,
					(GetSystemMetrics(SM_CXSCREEN) >> 1) - (WH_WIDTH >> 1),
					(GetSystemMetrics(SM_CYSCREEN) >> 1) - (WH_HEIGHT >> 1),
					WH_WIDTH + 16, WH_HEIGHT + 39, 0, 0, GetModuleHandleW(NULL), 0);
			else SetActiveWindow(help);
			break;
		case IR_HIDESHOW:
			isVisible = !isVisible;
			if (!console)
			{
				AddDebugConsole();
				printf("%s\n", LoadAndWriteTXT(IDT_TEXT1));
			}
			else ShowWindow(console, isVisible ? SW_SHOW : SW_HIDE);
			break;
		case IR_SYSSWITCH:
			isChangeLang = !isChangeLang;
			break;
		case IR_EXIT:
			isExit = TRUE;
			break;
		case IR_NEWVER:
			system("start https://github.com/ParadiseMan900/Fix-my-text/releases/latest");
		default:
			break;
		}
	}
	return DefWindowProcW(window, message, commandID, action);
}
LRESULT CALLBACK HelpReaction(HWND window, UINT message, WPARAM commandID, LPARAM action)
{
	static HBITMAP hBitMap = NULL;
	static unsigned xID = 0;
	switch (message)
	{
	case WM_CREATE:
		xID = 0;
		hBitMap = (HBITMAP)LoadImageW(GetModuleHandleW(NULL),
			MAKEINTRESOURCEW(IDB_BITMAP5), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		SetTimer(window, TID_INTRO, ANIM_INTERVAL, (TIMERPROC)NULL);
		break;
	case WM_TIMER:
		if (++xID == ANIM_SIZE)
		{
			KillTimer(window, TID_INTRO);
			xID--;
		}	
		else SendMessageW(window, WM_PAINT, 0, 0);
		return 0;
	case WM_PAINT:
	{
		HDC hdc = GetDC(window);
		HDC tempHdc = CreateCompatibleDC(hdc);
		SelectObject(tempHdc, hBitMap);
		BitBlt(hdc, 0, 0, WH_WIDTH, WH_HEIGHT, tempHdc, WH_WIDTH * xID, 0, SRCCOPY);
		DeleteDC(tempHdc);
		ReleaseDC(window, hdc);
	}
	break;
	case WM_CLOSE:
		DeleteObject(hBitMap);
		DestroyWindow(help);
		KillTimer(window, TID_INTRO);
		help = NULL;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(window, message, commandID, action);
}
void CallbackMessage()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		DispatchMessageW(&msg);
}
/////////////////////////////////////////////////////////////////////////////////////
void PrintBuferResult(struct StrData data, BOOL isGet)
{
	if (!console)
		return;
	if (isGet)
	{
		SetColor(SC_RED);
		printf("GetBufer:\t");
	}
	else
	{
		SetColor(SC_YELLOW);
		printf("SetBufer:\t");
	}
	SetColor(SC_WHITE);
	if (data.format == CF_UNICODETEXT)
		wprintf(L"\"%s\"\n", data.str);
	else
	{
		SetColor(SC_BLUE);
		printf("\"* PICTURE / КАРТИНКА *\"\n");
		SetColor(SC_WHITE);
	}
}
struct StrData GetBufer()
{
	struct StrData data = D_STRDATA;
	while (!OpenClipboard(0)) {}
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
		data.format = CF_UNICODETEXT;
	else if (IsClipboardFormatAvailable(CF_DIB))
		data.format = CF_DIB;
	else
	{
		CloseClipboard();
		return data;
	}
	HANDLE hData;
	hData = GetClipboardData(data.format);

	wchar_t* bufer = (wchar_t*)GlobalLock(hData);
	GlobalUnlock(hData);
	EmptyClipboard();
	CloseClipboard();
	data.size = GlobalSize(hData);
	if (bufer != NULL)
	{
		data.str = (wchar_t*)malloc(data.size);
		if (data.str != NULL)
			memcpy(data.str, bufer, data.size);
	}
	return data;
}
void SetBufer(struct StrData* data)
{
	HANDLE globAll = NULL;
	if (data->str != NULL)
	{
		globAll = GlobalAlloc(GMEM_DDESHARE, data->size);
		if (globAll)
		{
			void* globLock = GlobalLock(globAll);
			if (globLock)
				memcpy(globLock, data->str, data->size);
			GlobalUnlock(globAll);
		}

		while (!OpenClipboard(0)) {}
		EmptyClipboard();
		SetClipboardData(data->format, globAll);
		CloseClipboard();
		free(data->str);
	}
}
void LogMsg(const char* msg)
{
	if (!console)
		return;
	SetColor(SC_GREEN);
	printf("Message:\t%s\n", msg);
	SetColor(SC_WHITE);
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
BOOL LIsItEngWord(wchar_t* str, struct LanguageLib lib)
{
	int eng = 0;
	int other = 0;

	wchar_t letter;
	BOOL needToSkip = FALSE;
	for (int i = 0; i < wcslen(str); i++)
	{
		needToSkip = FALSE;
		letter = LDown(str[i]);

		for (int j = 0; j < lib.sizeSame; j++)
			if (letter == lib.sameChars[j])
			{
				needToSkip = TRUE;
				break;
			}
		if (needToSkip)
			continue;

		for (int j = 0; j < lib.sizeAll; j++)
			if (letter == lib.otherChars[j])
			{
				other++;
				needToSkip = TRUE;
				break;
			}
		if (needToSkip)
			continue;
		eng++;
	}
	if (other > eng)
		return FALSE;
	return TRUE;
}
wchar_t LSwap(wchar_t ch, BOOL eng, struct LanguageLib lib)
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
	struct LanguageLib RusLib = 
	{ L"`~@#$^&qwertyuiop[{]}|asdfghjkl;:'\"zxcvbnm,<.>/?" ,
	  L"ёЁ\"№;:?йцукенгшщзхХъЪ/фывапролджЖэЭячсмитьбБюЮ.,",
	  L"1234567890!%*()-_=+\\", 49 , 21 };

	BOOL isBig = FALSE;
	BOOL isEng = FALSE;

	size_t origLen = wcslen(str) + 1;
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
				isBig = FALSE;
				if (oneWord[j] != LDown(oneWord[j]))
					isBig = TRUE;
				oneWord[j] = LSwap(LDown(oneWord[j]), isEng, RusLib);
				if (isBig)
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
	for (int i = 0; i < wcslen(str); i++)
		if (str[i] == LUp(str[i]))
			str[i] = LDown(str[i]);
		else str[i] = LUp(str[i]);
}
void LogicSwitch(struct StrData* selected, int switcher)
{
	switch (switcher)
	{
	case LS_SWAP:
		LChanger(selected->str);
		LogMsg("Смена языка выполнена");
		break;
	case LS_UPDOWN:
		LSizer(selected->str);
		LogMsg("Смена регистра выполнена");
		break;
	default:
		return;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
int IsKeysPressed(const struct keysComb* comb)
{
	for (unsigned int i = 0; i < comb->len; i++)
		if (GetAsyncKeyState(comb->names[i]) >= 0) 
			return 0;
	return comb->ID;
}
void EmulateACombinationWithCtrl(wchar_t key)
{
	INPUT input[4];
	ZeroMemory(input, sizeof(input));
	for (int i = 0; i < 4; i++)
	{
		input[i].type = INPUT_KEYBOARD;
		input[i].ki.wVk = (i % 2) ? key : VK_CONTROL;
		if (i >= 2)
			input[i].ki.dwFlags = KEYEVENTF_KEYUP;
	}
	SendInput(4, input, sizeof(INPUT));
}
/////////////////////////////////////////////////////////////////////////////////////
BOOL IsNewVersionExist()
{
	HINTERNET hInterOpen = InternetOpenA("fmt", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hInterOpen)
		return FALSE;
	HINTERNET hInterconnect = InternetOpenUrlA(hInterOpen,
		"https://api.github.com/repos/ParadiseMan900/Fix-my-text/tags", NULL, 0, 0, 0);
	if (!hInterconnect)
		return FALSE;
	DWORD dataSize;
	InternetQueryDataAvailable(hInterconnect, &dataSize, 0, 0);
	char* str = (char*)malloc(sizeof(char) * (dataSize + 1));
	if(!str)
		return FALSE;
	str[dataSize] = '\0';
	InternetReadFile(hInterconnect, str, dataSize, &dataSize);
	InternetCloseHandle(hInterconnect);
	InternetCloseHandle(hInterOpen);
	BOOL result = FALSE;
	int verSize = 0;
	while (str[VERSIONID + verSize] != '\"')
		verSize++;
	char* version = (char*)malloc(sizeof(char) * (verSize + 1));
	if (version)
	{
		version[verSize] = '\0';
		for (int i = 0; i < verSize; i++)
			version[i] = str[VERSIONID + i];
		if (strcmp(version, VERSION))
			result = TRUE;
		free(version);
	}
	free(str);
	return result;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hInstPrev, _In_ char* cmdline, _In_ int cmdshow)
{
	//	Проверка на существование запущенной программы
	HANDLE hMutex = CreateMutexW(0, TRUE, L"FMT");
	if (!hMutex || GetLastError() == ERROR_ALREADY_EXISTS)
		return 1;

	////	Поиск, создание и регестрация окна, проверки, и т.п.
	NOTIFYICONDATA icon;
	ZeroMemory(&icon, sizeof(icon));
	{
		////	Регистрация класса (штука, чтобы "CALLBACK IconReaction" вызывалась)
		WNDCLASS wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = IconReaction;
		wc.hInstance = GetModuleHandleW(NULL);
		wc.lpszClassName = WCN_ICON;
		RegisterClassW(&wc);

		////	Создание и добавление окна (в виде иконки) в трей
		icon.cbSize = sizeof(icon);
		icon.hWnd = CreateWindowW(wc.lpszClassName, wc.lpszClassName, 0, 0, 0, 0, 0, 0, 0, wc.hInstance, 0);
		icon.uVersion = NOTIFYICON_VERSION;
		icon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		icon.uCallbackMessage = WM_USER;
		//IDI_ICON1 - ID Иконки из файла ресурсов.
		icon.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON1));
		const wchar_t* tip = L"Fix my text";
		wcscpy_s(icon.szTip, wcslen(tip) + 1, tip);


		////	Регистрация класса (штука, чтобы "CALLBACK HelpReaction" вызывалась)
		WNDCLASS wc2;
		ZeroMemory(&wc2, sizeof(wc2));
		wc2.lpfnWndProc = HelpReaction;
		wc2.hInstance = GetModuleHandleW(NULL);
		wc2.lpszClassName = WCN_HELP;
		wc2.hbrBackground = CreateSolidBrush(RGB(34, 30, 26));
		RegisterClassW(&wc2);
	}
	Shell_NotifyIconW(NIM_ADD, &icon);
	////	Список ключей клавиатуры:
	//https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

	////	ВСЕ ПЕРЕМЕННЫЕ 
	int toSWAP_M[] = { VK_LWIN, VK_LCONTROL };
	int toUPDOWN_M[] = { VK_LWIN, VK_LMENU };
	struct keysComb toSWAP = { 2, toSWAP_M, LS_SWAP };
	struct keysComb toUPDOWN = { 2,	toUPDOWN_M, LS_UPDOWN };

	struct StrData conservation = D_STRDATA;
	struct StrData selected = D_STRDATA;
	int switcher = 0;

	//Провкерка новой версии
	isNewVersion = IsNewVersionExist();
	unsigned long long ggg = 0;
	////	ЦАРЬ БАТЮШКА ЦИКЛ
	while (!isExit)
	{
		//printf("%lld\n", ggg++);
		if ((switcher = IsKeysPressed(&toSWAP)) || (switcher = IsKeysPressed(&toUPDOWN)))
		{
			if (console)
				printf("-----------------------------------------------\n");

			////	GET CONSERVATION;
			conservation = GetBufer();
			PrintBuferResult(conservation, TRUE);

			////	LOGIC (CTRL+C / GET / *DO STAF* / SET / CTRL+V);
			selected.str = NULL;

			//Проблемные кнопки, мешающие копированию
			while (GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0
				|| GetAsyncKeyState(VK_MENU) < 0)
			{
				CallbackMessage();
				Sleep(TS_PROCESSOR);
			}

			//Проверка на то что это не консоль (в консоли ctrl+c = смерть)
			if (console && GetForegroundWindow() == console)
			{
				LogMsg("Предотвращение ошибки. Не используйте FMT внутри консоли!");
				SetBufer(&conservation);
				continue;
			}

			//Эмуляция Ctrl+C
			EmulateACombinationWithCtrl('C');
			for (unsigned i = 0; i < TS_NORMAL; i++)
			{
				if ((selected = GetBufer()).str)
				{
					PrintBuferResult(selected, TRUE);
					break;
				}
				Sleep(TS_PROCESSOR);
			}
			//Скипаем логику если ничего не выделенно
			if (!selected.str)
			{
				PrintBuferResult(selected, TRUE);
				LogMsg("Ничего не выделенно");
				PrintBuferResult(conservation, FALSE);
				SetBufer(&conservation);
				continue;
			}
			if (selected.format == CF_DIB)
			{
				LogMsg("Пропуск картинки");
				PrintBuferResult(conservation, FALSE);
				SetBufer(&conservation);
				continue;
			}

			//Самая главная функция
			LogicSwitch(&selected, switcher);														   //<<<===
			PrintBuferResult(selected, FALSE);
			SetBufer(&selected);
			
			//Эмуляция Ctrl+V
			EmulateACombinationWithCtrl('V');
			Sleep(TS_SLOW_APP);
	
			////	SET CONSERVATION;
			PrintBuferResult(conservation, FALSE);
			SetBufer(&conservation);

			//Смена языка (при включеной галочки)
			if (isChangeLang && switcher == LS_SWAP)
				PostMessageW(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, 0, 0);
		}
		CallbackMessage();
		Sleep(TS_PROCESSOR);
	}
	//Dectructor
	DestroyWindow(icon.hWnd);
	Shell_NotifyIconW(NIM_DELETE, &icon);
	UnregisterClassW(WCN_ICON, GetModuleHandleW(NULL));
	UnregisterClassW(WCN_HELP, GetModuleHandleW(NULL));
	if (console)
		FreeConsole();
	ReleaseMutex(hMutex);
	return 0;
}