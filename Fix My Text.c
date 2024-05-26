//Code for "Fix my text" by Teslev Dmitry Sergeevich 2023 - 2024
#include "resource.h"
#include <windows.h>
#include <locale.h>
#include <stdio.h>
#include <wininet.h>
#pragma comment(lib, "WinInet.lib") 

//FMT Version data
#define	VERSION				"v6.0"
#define VERSIONID			10U
//Icon Reaction IDs
#define IR_HELP				0U
#define IR_HIDESHOW			1U
#define IR_SYSSWITCH		2U
#define IR_EXIT				3U
#define IR_NEWVER			4U
//Logic Switch IDs
#define LID_SWAP			1U
#define LID_UPDOWN			2U
//Set Color IDs
#define SC_GREEN			FOREGROUND_GREEN
#define SC_SKY				FOREGROUND_BLUE | FOREGROUND_GREEN
#define SC_RED				FOREGROUND_RED
#define SC_YELLOW			FOREGROUND_GREEN | FOREGROUND_RED
#define SC_WHITE			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
#define SC_NULL				SC_WHITE | BACKGROUND_RED
#define SetColor(colID)		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colID | FOREGROUND_INTENSITY)
//Time Sleep IDs
#define TS_PROCESSOR		5U
#define TS_NORMAL			15U
#define TS_SLOW_APP			250U
//Default struct data
#define D_STRDATA			{NULL, 0, 0}
//Сonstant case difference
#define CASE_DIFFERENCE		32U
//Window Class Name
#define WCN_ICON			L"TrayWin"
#define WCN_HELP			L"HELP"
//Help Window Size
#define WH_WIDTH			400U
#define WH_HEIGHT			250U
//Window Inaccuracy
#define WI_WIDTH			16U
#define WI_HEIGHT			39U
//Timer Settings
#define TID_PROBLEM			1U
#define TID_INTRO			2U
//Animation Settings
#define ANIM_SIZE			58U
#define ANIM_INTERVAL		5U

typedef unsigned char bool;

HWND consoleWin =			NULL;
HWND helpWin =				NULL;
HMODULE histance =			NULL;
bool isExit =				FALSE;
bool isVisible =			FALSE;
bool isChangeLang =			TRUE;
bool isNewVersion =			FALSE;
bool isProblemKeys =		FALSE;

struct StrData
{
	wchar_t* str;
	size_t byteSize;
	unsigned format;
};
/////////////////////////////////////////////////////////////////////////////////////
const char* LoadAndWriteTXT(int nameID)
{
	const char* data = "";
	HINSTANCE handle = histance;

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
HWND FindMainWindow(void)
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
void AddDebugConsole(void)
{
	AllocConsole();
	FILE* safeTemp;
	freopen_s(&safeTemp, "CON", "w", stdout);
	SetConsoleTitleW(L"Fix My Text - Debug mode");
	consoleWin = FindMainWindow();
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "");
	SetConsoleCtrlHandler(0, TRUE);
}
LRESULT CALLBACK IconReaction(HWND window, UINT message, WPARAM commandID, LPARAM action)
{
	switch (message)
	{
	case WM_CREATE:
		RegisterHotKey(window, LID_SWAP, MOD_CONTROL | MOD_WIN, 0);
		RegisterHotKey(window, LID_UPDOWN, MOD_ALT | MOD_WIN, 0);
		break;
	case WM_HOTKEY:
		SetTimer(window, TID_PROBLEM, TS_PROCESSOR, NULL);
		isProblemKeys = TRUE;
		break;
	case WM_TIMER:
		if (GetAsyncKeyState(VK_LWIN) >= 0 && GetAsyncKeyState(VK_RWIN) >= 0
			&& GetAsyncKeyState(VK_MENU) >= 0 && GetAsyncKeyState(VK_CONTROL) >= 0)
		{
			isProblemKeys = FALSE;
			KillTimer(window, TID_PROBLEM);
		}
		break;
	case WM_USER:
		//Создание менюшки с кнопками
		if (action == WM_RBUTTONUP || action == WM_LBUTTONUP || action == WM_MBUTTONDBLCLK)
		{
			HMENU hMenu = CreatePopupMenu();
			if (action == WM_MBUTTONDBLCLK)
			{
				AppendMenuW(hMenu, isVisible ? MF_CHECKED : MF_UNCHECKED, IR_HIDESHOW,
					L"Режим разработчика");
				SetMenuItemBitmaps(hMenu, IR_HIDESHOW, MF_BYCOMMAND,
					LoadBitmapW(histance, MAKEINTRESOURCEW(IDB_BITMAP3)), 0);
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
					LoadBitmapW(histance, MAKEINTRESOURCEW(IDB_BITMAP1)), 0);
				SetMenuItemBitmaps(hMenu, IR_SYSSWITCH, MF_BYCOMMAND,
					LoadBitmapW(histance, MAKEINTRESOURCEW(IDB_BITMAP4)), 0);
				SetMenuItemBitmaps(hMenu, IR_EXIT, MF_BYCOMMAND,
					LoadBitmapW(histance, MAKEINTRESOURCEW(IDB_BITMAP2)), 0);
			}
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(window);
			//TPM_BOTTOMALIGN == Меню правее курсора
			TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, window, 0);
			DestroyMenu(hMenu);
		}
		break;
	case WM_COMMAND:
		//Реакция на кнопки в меню
		switch (commandID)
		{
		case IR_HELP:
			////	Создание окна.
			if (!helpWin)
				helpWin = CreateWindowW(WCN_HELP, L"Помощь", WS_SYSMENU | WS_VISIBLE,
					(GetSystemMetrics(SM_CXSCREEN) >> 1) - (WH_WIDTH >> 1),
					(GetSystemMetrics(SM_CYSCREEN) >> 1) - (WH_HEIGHT >> 1),
					WH_WIDTH + WI_WIDTH, WH_HEIGHT + WI_HEIGHT, 0, 0, histance, 0);
			else SetActiveWindow(helpWin);
			break;
		case IR_HIDESHOW:
			isVisible = !isVisible;
			if (!consoleWin)
			{
				AddDebugConsole();
				printf("%s\n", LoadAndWriteTXT(IDT_TEXT1));
			}
			else ShowWindow(consoleWin, isVisible ? SW_SHOW : SW_HIDE);
			break;
		case IR_SYSSWITCH:
			isChangeLang = !isChangeLang;
			break;
		case IR_EXIT:
			isExit = TRUE;
			UnregisterHotKey(window, LID_SWAP);
			UnregisterHotKey(window, LID_UPDOWN);
			PostQuitMessage(0);
			break;
		case IR_NEWVER:
			system("start https://github.com/ParadiseMan900/Fix-my-text/releases/latest");
		default:
			break;
		}
		break;
	default:
		break;
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
		hBitMap = (HBITMAP)LoadImageW(histance,
			MAKEINTRESOURCEW(IDB_BITMAP5), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		SetTimer(window, TID_INTRO, ANIM_INTERVAL, (TIMERPROC)NULL);
		return 0;
	case WM_TIMER:
		xID == ANIM_SIZE - 1 ?	KillTimer(window, TID_INTRO) : xID++;
		InvalidateRect(window, NULL, TRUE);
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(window, &ps);
		HDC tempHdc = CreateCompatibleDC(hdc);
		SelectObject(tempHdc, hBitMap);
		BitBlt(hdc, 0, 0, WH_WIDTH, WH_HEIGHT, tempHdc, WH_WIDTH * xID, 0, SRCCOPY);
		DeleteDC(tempHdc);
		EndPaint(window, &ps);
		return 0;
	}
	case WM_CLOSE:
		DeleteObject(hBitMap);
		DestroyWindow(helpWin);
		KillTimer(window, TID_INTRO);
		helpWin = NULL;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(window, message, commandID, action);
}
/////////////////////////////////////////////////////////////////////////////////////
void LogMsg(const char* msg)
{
	if (!consoleWin)
		return;
	SetColor(SC_GREEN);
	printf("Message:\t%s\n", msg);
	SetColor(SC_WHITE);
}
void PrintBuferResult(struct StrData data, bool isGet)
{
	if (!consoleWin)
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
	
	switch (data.format)
	{
	case CF_UNICODETEXT:
		SetColor(SC_WHITE);
		wprintf(L"\"%s\"\n", data.str);
		break;
	case CF_DIB:
		SetColor(SC_SKY);
		printf("* PICTURE / КАРТИНКА *\n");
		SetColor(SC_WHITE);
		break;
	default:
		SetColor(SC_NULL);
		printf("|| NULL ||\n");
		SetColor(SC_WHITE);
		break;
	}
}
struct StrData GetBufer(void)
{
	struct StrData data = D_STRDATA;
	while (!OpenClipboard(0))
		Sleep(TS_PROCESSOR);
	if (!IsClipboardFormatAvailable(data.format = CF_UNICODETEXT)
		&& !IsClipboardFormatAvailable(data.format = CF_DIB))
	{
		CloseClipboard();
		data.format = 0;
		return data;
	}
	wchar_t* clipData = (wchar_t*)GetClipboardData(data.format);
	data.byteSize = (wcslen(clipData) + 1) * sizeof(wchar_t);
	data.str = malloc(data.byteSize);
	if (data.str)
		memcpy(data.str, clipData, data.byteSize);
	EmptyClipboard();
	CloseClipboard();
	return data;
}
void SetBufer(struct StrData* data)
{
	while (!OpenClipboard(0))
		Sleep(TS_PROCESSOR);
	EmptyClipboard();
	if (data->format)
	{
		HANDLE clipData = GlobalAlloc(GMEM_FIXED, data->byteSize);
		if (clipData)
			memcpy(clipData, data->str, data->byteSize);
		SetClipboardData(data->format, (HANDLE)clipData);
		free(data->str);
	}
	CloseClipboard();
}
/////////////////////////////////////////////////////////////////////////////////////
wchar_t LUp(wchar_t letter)
{
	if ((letter >= L'a' && letter <= L'z')
		|| (letter >= L'а' && letter <= L'я'))
		letter -= CASE_DIFFERENCE;
	if (letter == L'ё')
		letter = L'Ё';
	return letter;
}
wchar_t LDown(wchar_t letter)
{
	if ((letter >= L'A' && letter <= L'Z')
		|| (letter >= L'А' && letter <= L'Я'))
		letter += CASE_DIFFERENCE;
	if (letter == L'Ё')
		letter = L'ё';
	return letter;
}
bool LIsCharInTheLine(const wchar_t* str, size_t byteSize, wchar_t wchar)
{
	for (size_t i = 0; i < byteSize; i++)
		if (str[i] == wchar)
			return TRUE;
	return FALSE;
}
void LogicSwitch(struct StrData* opt, int switcher)
{
	switch (switcher)
	{
	case LID_SWAP:
	{
		const wchar_t* engChars = L"`~@#$^&qQwWeErRtTyYuUiIoOpP[{]}|aAsSdDfFgGhHjJkKlL;:'\"zZxXcCvVbBnNmM,<.>/?";
		const wchar_t* rusChars = L"ёЁ\"№;:?йЙцЦуУкКеЕнНгГшШщЩзЗхХъЪ/фФыЫвВаАпПрРоОлЛдДжЖэЭяЯчЧсСмМиИтТьЬбБюЮ.,";
		const wchar_t* sameChars = L"1234567890!%*()-_=+\\\";:?.,/";
		size_t swapCharsSize = wcslen(engChars);
		size_t sameCharsSize = wcslen(sameChars);
		size_t strSize = wcslen(opt->str) + 1;
		unsigned engCount;
		unsigned rusCount;
		wchar_t* oneWord;
		size_t wordLen;
		size_t lastID = 0;
		for (size_t i = 0; i < strSize; i++)
			if (opt->str[i] == L' ' || opt->str[i] == L'\t' || opt->str[i] == L'\n' || opt->str[i] == L'\0')
			{
				wordLen = (size_t)i - lastID;
				oneWord = opt->str + lastID;
				engCount = 0;
				rusCount = 0;
				for (size_t j = 0; j < wordLen; j++)
				{
					if (LIsCharInTheLine(sameChars, sameCharsSize, oneWord[j]))
						continue;
					if (LIsCharInTheLine(engChars, swapCharsSize, oneWord[j]))
						engCount++;
					else rusCount++;
				}
				if (engCount >= rusCount)
				{
					for (size_t j = 0; j < wordLen; j++)
						for (size_t k = 0; k < swapCharsSize; k++)
							if (engChars[k] == oneWord[j])
							{
								oneWord[j] = rusChars[k];
								break;
							}
				}
				else
					for (size_t j = 0; j < wordLen; j++)
						for (size_t k = 0; k < swapCharsSize; k++)
							if (rusChars[k] == oneWord[j])
							{
								oneWord[j] = engChars[k];
								break;
							}
				lastID = i + 1;
			}
		LogMsg("Смена языка выполнена");
	}
		break;
	case LID_UPDOWN:
	{
		wchar_t upChar;
		size_t strSize = wcslen(opt->str);
		for (size_t i = 0; i < strSize; i++)
		{
			upChar = LUp(opt->str[i]);
			opt->str[i] = opt->str[i] == upChar ? LDown(opt->str[i]) : upChar;
		}
		LogMsg("Смена регистра выполнена");
	}
		break;
	default:
		return;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void EmulateACombinationWithCtrl(wchar_t key)
{
	INPUT input[4];
	ZeroMemory(input, sizeof(input));
	input[0].ki.wVk = VK_CONTROL;
	input[1].ki.wVk = key;
	input[1].ki.time = TS_PROCESSOR;
	input[2].ki.wVk = key;
	input[2].ki.dwFlags = KEYEVENTF_KEYUP;
	input[3].ki.wVk = VK_CONTROL;
	input[3].ki.dwFlags = KEYEVENTF_KEYUP;
	input[3].ki.time = TS_PROCESSOR;
	for (unsigned i = 0; i < 4; i++)
		input[i].type = INPUT_KEYBOARD;
	UINT result = SendInput(4, input, sizeof(INPUT));
	if (result != 4)
		LogMsg("Emulate ERROR");
}
/////////////////////////////////////////////////////////////////////////////////////
bool IsNewVersionExist(void)
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
	bool result = FALSE;
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
	histance = GetModuleHandleW(NULL);
	NOTIFYICONDATA icon;
	ZeroMemory(&icon, sizeof(icon));
	{
		////	Регистрация класса (штука, чтобы "CALLBACK IconReaction" вызывалась)
		WNDCLASS wc;	
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = IconReaction;
		wc.hInstance = histance;
		wc.lpszClassName = WCN_ICON;
		RegisterClassW(&wc);

		////	Создание и добавление окна (в виде иконки) в трей
		icon.cbSize = sizeof(icon);
		icon.hWnd = CreateWindowW(wc.lpszClassName, wc.lpszClassName, 0, 0, 0, 0, 0, 0, 0, wc.hInstance, 0);
		icon.uVersion = NOTIFYICON_VERSION;
		icon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		icon.uCallbackMessage = WM_USER;
		//IDI_ICON1 - ID Иконки из файла ресурсов.
		icon.hIcon = LoadIconW(histance, MAKEINTRESOURCEW(IDI_ICON1));
		const wchar_t* tip = L"Fix my text";
		wcscpy_s(icon.szTip, wcslen(tip) + 1, tip);

		////	Регистрация класса (штука, чтобы "CALLBACK HelpReaction" вызывалась)
		WNDCLASS wc2;
		ZeroMemory(&wc2, sizeof(wc2));
		wc2.lpfnWndProc = HelpReaction;
		wc2.hInstance = histance;
		wc2.lpszClassName = WCN_HELP;
		wc2.hbrBackground = CreateSolidBrush(RGB(34, 30, 26));
		wc2.hIcon = LoadIconW(histance, MAKEINTRESOURCEW(IDI_ICON1));
		RegisterClassW(&wc2);
	}
	Shell_NotifyIconW(NIM_ADD, &icon);

	////	Список ключей клавиатуры:
	//https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

	////	ВСЕ ПЕРЕМЕННЫЕ 
	struct StrData conservation = D_STRDATA, selected = D_STRDATA;
	unsigned switcher = 0;

	//Провкерка новой версии
	isNewVersion = IsNewVersionExist();

	////	ЦАРЬ БАТЮШКА ЦИКЛ
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (!isExit)
		if (GetMessageW(&msg, NULL, 0, 0))
		{
			DispatchMessageW(&msg);
			if (msg.message == WM_HOTKEY)
			{
				switcher = (unsigned)msg.wParam;
				if (consoleWin)
					printf("-----------------------------------------------\n");

				////	GET CONSERVATION;
				PrintBuferResult(conservation = GetBufer(), TRUE);

				////	LOGIC (CTRL+C / GET / *DO STAF* / SET / CTRL+V);
				selected.str = NULL;

				////Проблемные кнопки, мешающие копированию
				while (isProblemKeys)
					if (GetMessageW(&msg, NULL, 0, 0))
						DispatchMessageW(&msg);

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

				while (!GetOpenClipboardWindow()) {}
				while (GetOpenClipboardWindow()) { Sleep(TS_PROCESSOR); }
				//for (; ;)
				//	if (GetOpenClipboardWindow())
				//	{
				//		printf("gandon\n");
				//		break;
				//	}
						

				////	SET CONSERVATION;
				PrintBuferResult(conservation, FALSE);
				SetBufer(&conservation);
		
				//Смена языка (при включеной галочки)
				if (isChangeLang && switcher == LID_SWAP)
					PostMessageW(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, 0, 0);
			}
		}
	//Dectructor
	DestroyWindow(icon.hWnd);
	Shell_NotifyIconW(NIM_DELETE, &icon);
	UnregisterClassW(WCN_ICON, histance);
	UnregisterClassW(WCN_HELP, histance);
	if (consoleWin)
		FreeConsole();
	ReleaseMutex(hMutex);
	return 0;
}