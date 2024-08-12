//Code for "Fix my text" by Teslev Dmitry Sergeevich 2023 - 2024
#include "resource.h"
#include <windows.h>
#include <locale.h>
#include <stdio.h>
#include <wininet.h>
#pragma comment(lib, "WinInet.lib")
#pragma comment(lib, "winmm.lib")
#pragma warning( disable : 4554 ) // Я знаю как работают >> и <<
#define VAR_NAME(name)			#name

//FMT Version data
#define	VERSION					"v6.3"
#define VERSIONID				10U
//Icon Reaction IDs
#define IR_HELP					1U
#define IR_HIDESHOW				2U
#define IR_SYSSWITCH			3U
#define IR_EXIT					4U
#define IR_NEWVER				5U
#define IR_WEB_GOOGLE			6U
#define IR_WEB_YANDEX			7U
//Logic Switch IDs
#define LID_SWAP				1U
#define LID_UPDOWN				2U
#define LID_TOWEB				3U
//Set Color IDs
#define SC_GREEN				FOREGROUND_GREEN
#define SC_SKY					FOREGROUND_BLUE | FOREGROUND_GREEN
#define SC_RED					FOREGROUND_RED
#define SC_YELLOW				FOREGROUND_GREEN | FOREGROUND_RED
#define SC_WHITE				FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
#define SC_NULL					SC_WHITE | BACKGROUND_RED
#define SetColor(colID)			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
									colID | FOREGROUND_INTENSITY)
//Сonstant case difference
#define CASE_DIFFERENCE			32U
//Window Class Name
#define WCN_ICON				L"TrayWin"
#define WCN_HELP				L"HELP"
//Help Window Size
#define WH_WIDTH				400U
#define WH_HEIGHT				250U
//Window Inaccuracy
#define WI_WIDTH				16U
#define WI_HEIGHT				39U
//Timer Settings
#define TID_PROBLEM				1U
#define TID_CTRLC				2U
#define TID_CTRLV				3U
#define TID_NEWVER				4U
//Animation Settings
#define ANIM_SIZE				60U		//60 кадров
#define ANIM_INTERVAL			16U		//Время анимации - 1 сек в 60 кадров. 1000/60 = 16
//New version website
#define GOTO_GITHUB				"start https://github.com/ParadiseMan900/Fix-my-text/releases/latest"
//Settings name
#define SET_FILE				"FMT_Settings.txt"

typedef unsigned char			bool;

union BoolSettings
{
	bool memSell;
	struct
	{
		bool isVisible : 1;
		bool isChangeLang : 1;
		bool isNewVersion : 1;
		bool isProblemKeys : 1;
		bool isWaitingClipboard : 1;
		bool isWaitingInput : 1;
		bool isDebugMode : 1;
		bool isGoogle : 1;
	};
};
struct StrData
{
	wchar_t* str;
	size_t byteSize;
	size_t strSize;
	USHORT format;
};

HWND helpWin =					NULL;
HWND consoleWin =				NULL;
HMODULE histance =				NULL;
unsigned frameID =				0U;
union BoolSettings settings =	{ 0, .isChangeLang = TRUE };
NOTIFYICONDATAW nTray;


/////////////////////////////////////////////////////////////////////////////////////
bool IsNewVersionExist(void)
{
	HINTERNET hInterOpen = InternetOpenW(L"fmt", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hInterOpen)
		return FALSE;
	HINTERNET hInterconnect = InternetOpenUrlW(hInterOpen,
		L"https://api.github.com/repos/ParadiseMan900/Fix-my-text/tags", NULL, 0, 0, 0);
	if (!hInterconnect)
		return FALSE;
	DWORD dataSize = 0;
	InternetQueryDataAvailable(hInterconnect, &dataSize, 0, 0);
	char* str = calloc((size_t)dataSize + 1, sizeof(char));
	if (!str)
		return FALSE;
	InternetReadFile(hInterconnect, str, dataSize, &dataSize);
	InternetCloseHandle(hInterconnect);
	InternetCloseHandle(hInterOpen);
	bool result = FALSE;
	size_t verSize = 0;
	while (str[VERSIONID + verSize] != '\"')
		verSize++;
	char* version = calloc(verSize + 1, sizeof(char));
	if (version)
	{
		for (size_t i = 0; i != verSize; i++)
			version[i] = str[VERSIONID + i];
		if (strcmp(version, VERSION))
		{
			result = TRUE;
			Shell_NotifyIconW(NIM_MODIFY, &nTray);
		}

		free(version);
	}
	free(str);
	return result;
}
const char* LoadAndWriteTXT(int nameID)
{
	const char* data = 0;
	//Ссылка на ресурс
	HRSRC rcNameID = FindResourceW(histance, MAKEINTRESOURCEW(nameID), MAKEINTRESOURCEW(TEXTFILE));
	if (rcNameID)
	{
		//Данные в TXT (Все)
		HANDLE rcData = LoadResource(histance, rcNameID);
		if (rcData)
			data = LockResource(rcData);
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
	HMENU hMenu = GetSystemMenu(consoleWin, FALSE);
	if (hMenu) 
		DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
}
HWND CreateClientWindow(WCHAR* lpClassName, WCHAR* lpWindowName, DWORD dwStyle, int X, int Y,
	int nWidth, int nHeight, HINSTANCE hInstance)
{
	RECT rect = { X, Y, nWidth + X, nHeight + Y };
	AdjustWindowRect(&rect, dwStyle, FALSE);
	HWND resultHWND = CreateWindowW(lpClassName, lpWindowName, dwStyle, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	return resultHWND;
}
LRESULT CALLBACK IconReaction(HWND window, UINT message, WPARAM commandID, LPARAM action)
{
	switch (message)
	{
	case WM_CREATE:
		RegisterHotKey(window, LID_SWAP, MOD_CONTROL | MOD_WIN, 0);
		RegisterHotKey(window, LID_UPDOWN, MOD_ALT | MOD_WIN, 0);
		RegisterHotKey(window, LID_TOWEB, MOD_CONTROL | MOD_ALT, 0);
		break;
	case WM_TIMER:
		switch (commandID)
		{
		case TID_PROBLEM:
			if (GetKeyState(VK_LWIN) >= 0 && GetKeyState(VK_RWIN) >= 0
				&& GetKeyState(VK_MENU) >= 0 && GetKeyState(VK_CONTROL) >= 0)
					settings.isProblemKeys = FALSE;
			break;
		case TID_CTRLC:
		{
			static unsigned char iterNULL = 0;
			if (++iterNULL == 15)	//15 итераций проверки
			{
				settings.isWaitingClipboard = FALSE;
				iterNULL = 0;
			}
			break;
		}
		case TID_CTRLV:
			for (unsigned char i = 0; i < 255; i++) 
				if (GetKeyState(i) < 0)
				{
					settings.isWaitingInput = FALSE;
					break;
				}
			break;
		case TID_NEWVER:
			if (settings.isNewVersion = IsNewVersionExist())
				KillTimer(window, TID_NEWVER);
			break;
		default:
			break;
		}
		break;
	case WM_CLIPBOARDUPDATE:
		settings.isWaitingClipboard = FALSE;
		break;
	case WM_USER:
		switch (action)
		{
		case WM_RBUTTONUP: case WM_LBUTTONUP:
		{
			//Создание менюшки с кнопками
			HMENU hMenu = CreatePopupMenu();
			if (settings.isNewVersion)
			{
				AppendMenuW(hMenu, 0, IR_NEWVER, L"Доступна новая версия!");
				AppendMenuW(hMenu, MF_SEPARATOR, 0, 0);
			}
			AppendMenuW(hMenu, 0, IR_HELP, L"Помощь");
			AppendMenuW(hMenu, settings.isChangeLang ? MF_CHECKED : MF_UNCHECKED, IR_SYSSWITCH,
				L"Автоматически менять язык системы");
			HMENU hMenuWeb = CreatePopupMenu();
			AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuWeb, L"Поисковая система");
			UINT radio[2] = { 0, 0 };
			radio[settings.isGoogle ? 0 : 1] = MF_CHECKED | MF_GRAYED;
			AppendMenuW(hMenuWeb, radio[0], IR_WEB_GOOGLE, L"Google");
			AppendMenuW(hMenuWeb, radio[1], IR_WEB_YANDEX, L"Яндекс");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
			if (settings.isDebugMode)
			{
				AppendMenuW(hMenu, settings.isVisible ? MF_CHECKED : MF_UNCHECKED, IR_HIDESHOW,
					L"Режим разработчика");
				AppendMenuW(hMenu, MF_SEPARATOR, 0, 0);

				SetMenuItemBitmaps(hMenu, IR_HIDESHOW, MF_BYCOMMAND,
					LoadImageW(histance, MAKEINTRESOURCEW(IDB_BITMAP3), IMAGE_BITMAP, 0, 0, LR_SHARED), 0);
			}
			AppendMenuW(hMenu, 0, IR_EXIT, L"Выход");

			SetMenuItemBitmaps(hMenu, IR_HELP, MF_BYCOMMAND,
				LoadImageW(histance, MAKEINTRESOURCEW(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_SHARED), 0);
			SetMenuItemBitmaps(hMenu, IR_SYSSWITCH, MF_BYCOMMAND,
				LoadImageW(histance, MAKEINTRESOURCEW(IDB_BITMAP4), IMAGE_BITMAP, 0, 0, LR_SHARED), 0);
			SetMenuItemBitmaps(hMenu, IR_EXIT, MF_BYCOMMAND,
				LoadImageW(histance, MAKEINTRESOURCEW(IDB_BITMAP2), IMAGE_BITMAP, 0, 0, LR_SHARED), 0);
			SetMenuItemBitmaps(hMenu, IR_WEB_GOOGLE, MF_BYCOMMAND,
				LoadImageW(histance, MAKEINTRESOURCEW(IDB_BITMAP6), IMAGE_BITMAP, 0, 0, LR_SHARED), 0);
			SetMenuItemBitmaps(hMenu, IR_WEB_YANDEX, MF_BYCOMMAND,
				LoadImageW(histance, MAKEINTRESOURCEW(IDB_BITMAP7), IMAGE_BITMAP, 0, 0, LR_SHARED), 0);

			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(window);
			//TPM_BOTTOMALIGN == Меню правее курсора
			TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, window, 0);
			DestroyMenu(hMenu);
			break;
		}
		case WM_MBUTTONDBLCLK:		settings.isDebugMode = TRUE;	break;
		case NIN_BALLOONUSERCLICK:	system(GOTO_GITHUB);			break;
		default:													break;
		}
		break;
	case WM_COMMAND:
		//Реакция на кнопки в меню
		switch (commandID)
		{
		case IR_HELP:
			if (!helpWin)
				helpWin = CreateClientWindow(WCN_HELP, L"Помощь", WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
					(GetSystemMetrics(SM_CXSCREEN) >> 1) - (WH_WIDTH >> 1),
					(GetSystemMetrics(SM_CYSCREEN) >> 1) - (WH_HEIGHT >> 1),
					WH_WIDTH, WH_HEIGHT, histance);
			else SetActiveWindow(helpWin);
			break;
		case IR_HIDESHOW:
			settings.isVisible = !settings.isVisible;
			if (!consoleWin)
			{
				AddDebugConsole();
				printf("%s\n", LoadAndWriteTXT(IDT_TEXT1));
			}
			else ShowWindow(consoleWin, settings.isVisible ? SW_SHOW : SW_HIDE);
			break;
		case IR_SYSSWITCH:
			settings.isChangeLang = !settings.isChangeLang;
			break;
		case IR_EXIT:
			UnregisterHotKey(window, LID_SWAP);
			UnregisterHotKey(window, LID_UPDOWN);
			UnregisterHotKey(window, LID_TOWEB);
			if(helpWin)
				SendMessageW(helpWin, WM_CLOSE, 0, 0);
			PostQuitMessage(0);
			break;
		case IR_NEWVER:
			system(GOTO_GITHUB);
		case IR_WEB_GOOGLE: case IR_WEB_YANDEX:
			settings.isGoogle = !settings.isGoogle;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return DefWindowProcW(window, message, commandID, action);
}
void CALLBACK TimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (frameID != ANIM_SIZE - 1)
	{
		frameID++;
		InvalidateRect(helpWin, NULL, TRUE);
		UpdateWindow(helpWin);
	}
	else timeKillEvent(uTimerID);
}
LRESULT CALLBACK HelpReaction(HWND window, UINT message, WPARAM commandID, LPARAM action)
{
	static HBITMAP hBitMap = NULL;
	static MMRESULT animTimer = 0;
	switch (message)
	{
	case WM_CREATE:
		frameID = 0;
		hBitMap = LoadImageW(histance, MAKEINTRESOURCEW(IDB_BITMAP5), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		animTimer = timeSetEvent(ANIM_INTERVAL, 0, TimerCallback, 0, TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(window, &ps);
		HDC tempHdc = CreateCompatibleDC(hdc);
		SelectObject(tempHdc, hBitMap);
		BitBlt(hdc, 0, 0, WH_WIDTH, WH_HEIGHT, tempHdc, WH_WIDTH * frameID, 0, SRCCOPY);
		DeleteDC(tempHdc);
		EndPaint(window, &ps);
		return 0;
	}
	case WM_CLOSE:
		DeleteObject(hBitMap);
		DestroyWindow(helpWin);
		helpWin = NULL;
		if(frameID != ANIM_SIZE - 1)
			timeKillEvent(animTimer);
		return 0;
	}
	return DefWindowProcW(window, message, commandID, action);
}
/////////////////////////////////////////////////////////////////////////////////////
void LogMsg(const char* msg, WORD msgColor)
{
	if (!consoleWin)
		return;
	SetColor(msgColor);
	printf("/#/ %s /#/\n", msg);
	SetColor(SC_WHITE);
}
void PrintStrData(struct StrData* data, const char* msg)
{
	if (!consoleWin)
		return;
	SetColor(SC_RED);
	printf("%s[%llu] -> ", msg, data->strSize);
	switch (data->format)
	{
	case CF_UNICODETEXT:
		SetColor(SC_WHITE);
		wprintf(L"\"%s\"\n", data->str);
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
	struct StrData data;
	ZeroMemory(&data, sizeof(struct StrData));
	while (!OpenClipboard(0))
		Sleep(USER_TIMER_MINIMUM);
	LogMsg("Call GetBufer", SC_YELLOW);
	if (!IsClipboardFormatAvailable(data.format = CF_UNICODETEXT)
		&& !IsClipboardFormatAvailable(data.format = CF_DIB))
	{
		CloseClipboard();
		data.format = 0;
		return data;
	}
	wchar_t* clipData = (wchar_t*)GetClipboardData(data.format);
	if (data.format == CF_UNICODETEXT)
		data.strSize = wcslen(clipData) + 1;
	data.byteSize = GlobalSize(clipData);
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
		Sleep(USER_TIMER_MINIMUM);
	EmptyClipboard();
	if (data->format)
	{
		HANDLE clipData = GlobalAlloc(GMEM_FIXED, data->byteSize);
		if (clipData)
			memcpy(clipData, data->str, data->byteSize);
		SetClipboardData(data->format, clipData);
		free(data->str);
		data->str = NULL;
		data->strSize = 0;
		data->byteSize = 0;
	}
	CloseClipboard();
	LogMsg("Call SetBufer", SC_YELLOW);
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
bool LIsCharInTheLine(const wchar_t* str, size_t strSize, wchar_t wchar)
{
	for (size_t i = 0; i < strSize; i++)
		if (str[i] == wchar)
			return TRUE;
	return FALSE;
}
void LogicSwitch(struct StrData* opt, unsigned switcher)
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
		unsigned engCount;
		unsigned rusCount;
		wchar_t* oneWord;
		size_t wordLen;
		size_t lastID = 0;
		for (size_t i = 0; i < opt->strSize; i++)
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
		LogMsg("Смена языка выполнена", SC_GREEN);
		break;
	}
	case LID_UPDOWN:
	{
		wchar_t upChar;
		for (size_t i = 0; i < opt->strSize - 1; i++)
		{
			upChar = LUp(opt->str[i]);
			opt->str[i] = opt->str[i] == upChar ? LDown(opt->str[i]) : upChar;
		}
		LogMsg("Смена регистра выполнена", SC_GREEN);
		break;
	}
	case LID_TOWEB:
	{
		USHORT plusCounter = 0;			//+ = %2B
		for (size_t i = 0; i < opt->strSize - 1; i++)
			if (opt->str[i] == '+')
				plusCounter++;
		wchar_t* strCopy = calloc((opt->strSize + (size_t)plusCounter * 2), sizeof(wchar_t));
		if (strCopy)
		{
			for (size_t i = 0, j = 0; i < opt->strSize - 1; i++, j++)
				switch (opt->str[i])
				{
				case L' ':	case L'\t':	strCopy[j] = L'+';												break;
				case L'+':	strCopy[j] = L'%';	strCopy[j + 1] = L'2'; strCopy[j + 2] = L'B'; j += 2;	break;
				default:	strCopy[j] = opt->str[i];													break;
				}

			const wchar_t* emptyLink = 	settings.isGoogle ?
				L"start https://www.google.ru/search?q=" : L"start https://yandex.ru/search/?text=";
			size_t linkSize = wcslen(emptyLink) + wcslen(strCopy) + 1;
			wchar_t* link = calloc(linkSize, sizeof(wchar_t));
			if (link)
			{
				wcscpy_s(link, linkSize, emptyLink);
				wcscat_s(link, linkSize, strCopy);
				_wsystem(link);
				free(link);
				LogMsg("Передача текста в браузер выполнена", SC_GREEN);
			}
			free(strCopy);
		}
		break;
	}
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
	input[2].ki.wVk = key;
	input[2].ki.dwFlags = KEYEVENTF_KEYUP;
	input[3].ki.wVk = VK_CONTROL;
	input[3].ki.dwFlags = KEYEVENTF_KEYUP;
	for (unsigned i = 0; i < 4; i++)
		input[i].type = INPUT_KEYBOARD;
	if (SendInput(4, input, sizeof(INPUT)) != 4)
		LogMsg("Emulate ERROR", SC_GREEN);
}
/////////////////////////////////////////////////////////////////////////////////////
int WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hInstPrev, _In_ WCHAR* cmdLine, _In_ int cmdShow)
{
	////	Проверка на существование запущенной программы
	HANDLE hMutex = CreateMutexW(0, TRUE, L"FMT");
	if (!hMutex || GetLastError() == ERROR_ALREADY_EXISTS)
		return 1;

	//// Импорт настроек
	{
		SetFileAttributesA(SET_FILE, FILE_ATTRIBUTE_NORMAL);
		FILE* fSettings;
		fopen_s(&fSettings, "FMT_Settings.txt", "r");
		if (fSettings)
		{
			bool scanValue[2] = { 0, 0 };
			int scanCode = fscanf_s(fSettings, "%hhu %hhu", &scanValue[0], &scanValue[1]);
			if (scanCode && scanCode != EOF)
			{
				settings.isChangeLang = scanValue[0];
				settings.isGoogle = scanValue[1];
			}
			fclose(fSettings);
			SetFileAttributesA(SET_FILE, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
		}
	}

	////	Реакция на атрибуты
	{
		int argsCount;
		wchar_t** cmdArgvLine;
		if (cmdArgvLine = CommandLineToArgvW(cmdLine, &argsCount))
			for (int i = 0; i != argsCount; i++)
				if (!lstrcmpW(cmdArgvLine[i], L"debug"))
				{
					settings.isDebugMode = TRUE;
					break;
				}
	}

	////	Поиск, создание и регестрация окна, настройки
	histance = hInst;
	{
		////	Регистрация классов
		WNDCLASS wc;	
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = IconReaction;
		wc.hInstance = histance;
		wc.lpszClassName = WCN_ICON;
		RegisterClassW(&wc);

		WNDCLASS wc2;
		ZeroMemory(&wc2, sizeof(wc2));
		wc2.lpfnWndProc = HelpReaction;
		wc2.hInstance = histance;
		wc2.lpszClassName = WCN_HELP;
		wc2.hIcon = LoadImageW(histance, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 0, 0,	LR_DEFAULTSIZE);
		RegisterClassW(&wc2);

		////	Создание и добавление окна (в виде иконки) в трей
		ZeroMemory(&nTray, sizeof(nTray));
		nTray.cbSize = sizeof(nTray);
		nTray.hWnd = CreateWindowW(wc.lpszClassName, wc.lpszClassName, 0, 0, 0, 0, 0, 0, 0, wc.hInstance, 0);
		nTray.uVersion = NOTIFYICON_VERSION;
		nTray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nTray.uCallbackMessage = WM_USER;
		//IDI_ICON1 - ID Иконки из файла ресурсов.
		nTray.hIcon = LoadImageW(histance, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
		lstrcpyW(nTray.szTip, L"Fix my text");
		nTray.hBalloonIcon = LoadImageW(histance, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 0, 0, 0);
		nTray.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON | NIIF_RESPECT_QUIET_TIME;
		lstrcpyW(nTray.szInfoTitle, L"Появилась новая версия приложения!");
		lstrcpyW(nTray.szInfo, L"Нажмите на иконку, чтобы перейти на страницу с новой версией приложения");
		Shell_NotifyIconW(NIM_ADD, &nTray);
		nTray.uFlags |= NIF_INFO;
		DestroyIcon(nTray.hIcon);
		nTray.hIcon = LoadImageW(histance, MAKEINTRESOURCEW(IDI_ICON2), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	}

	////	Провкерка новой версии
	if (!(settings.isNewVersion = IsNewVersionExist()))
		SetTimer(nTray.hWnd, TID_NEWVER, 600'000, NULL);	//Каждые 10 минут

	////	ВСЕ ПЕРЕМЕННЫЕ 
	struct StrData conservation, selected;
	ZeroMemory(&conservation, sizeof(struct StrData));
	ZeroMemory(&selected, sizeof(struct StrData));

	////	ЦАРЬ БАТЮШКА ЦИКЛ
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		DispatchMessageW(&msg);
		if (msg.message != WM_HOTKEY)
			continue;
		unsigned swither = (unsigned)msg.wParam;
		if (consoleWin)
			printf("--------------------------------------------------------------------------------------\n");

		////	Получение CONSERVATION
		conservation = GetBufer();
		PrintStrData(&conservation, VAR_NAME(conservation));

		////	Проблемные кнопки, мешающие копированию
		SetTimer(nTray.hWnd, TID_PROBLEM, USER_TIMER_MINIMUM, NULL);
		settings.isProblemKeys = TRUE;
		while (settings.isProblemKeys)
			if (GetMessageW(&msg, NULL, 0, 0))
				DispatchMessageW(&msg);
		KillTimer(nTray.hWnd, TID_PROBLEM);

		////	Эмуляция Ctrl+C
		AddClipboardFormatListener(nTray.hWnd);
		SetTimer(nTray.hWnd, TID_CTRLC, USER_TIMER_MINIMUM, NULL);
		settings.isWaitingClipboard = TRUE;
		EmulateACombinationWithCtrl('C');
		while (settings.isWaitingClipboard)
			if (GetMessageW(&msg, NULL, 0, 0))
				DispatchMessageW(&msg);
		KillTimer(nTray.hWnd, TID_CTRLC);
		RemoveClipboardFormatListener(nTray.hWnd);

		////	Получение SELECTED
		selected = GetBufer();
		PrintStrData(&selected, VAR_NAME(selected));

		////	Пропускаем логику если ничего не выделенно
		if (!selected.str)
		{
			LogMsg("Ничего не выделенно", SC_GREEN);
			SetBufer(&conservation);
			continue;
		}
		if (selected.format == CF_DIB)
		{
			LogMsg("Пропуск картинки", SC_GREEN);
			SetBufer(&conservation);
			continue;
		}

		////	Самая главная функция																		<<<===
		LogicSwitch(&selected, swither);
		if (swither == LID_SWAP || swither == LID_UPDOWN)
		{
			PrintStrData(&selected, VAR_NAME(selected));
			SetBufer(&selected);

			////	Эмуляция Ctrl+V
			EmulateACombinationWithCtrl('V');

			////	Ожидания действий для корректной вставки CONSERVATION
			SetTimer(nTray.hWnd, TID_CTRLV, USER_TIMER_MINIMUM, NULL);
			settings.isWaitingInput = TRUE;
			while (settings.isWaitingInput)
				if (GetMessageW(&msg, NULL, 0, 0))
					DispatchMessageW(&msg);
			KillTimer(nTray.hWnd, TID_CTRLV);
		}

		////	Вставка CONSERVATION
		SetBufer(&conservation);

		////	Смена языка (при включеной галочки)
		if (settings.isChangeLang && swither == LID_SWAP)
			PostMessageW(GetTopWindow(GetDesktopWindow()), WM_INPUTLANGCHANGEREQUEST, 0, 0);
	}
	//// Экспорт настроек
	SetFileAttributesA(SET_FILE, FILE_ATTRIBUTE_NORMAL);
	FILE* fSettings;
	fopen_s(&fSettings, SET_FILE, "w");
	if (fSettings)
	{
		fprintf_s(fSettings, "%hhu %hhu", settings.isChangeLang, settings.isGoogle);
		fclose(fSettings);
		SetFileAttributesA(SET_FILE, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
	}
	////	Очитска
	KillTimer(nTray.hWnd, TID_NEWVER);
	DestroyWindow(nTray.hWnd);
	Shell_NotifyIconW(NIM_DELETE, &nTray);
	UnregisterClassW(WCN_ICON, histance);
	UnregisterClassW(WCN_HELP, histance);
	if (consoleWin)
		FreeConsole();
	ReleaseMutex(hMutex);
	return 0;
}