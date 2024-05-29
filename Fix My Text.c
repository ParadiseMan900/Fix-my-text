//Code for "Fix my text" by Teslev Dmitry Sergeevich 2023 - 2024
#include "resource.h"
#include <windows.h>
#include <locale.h>
#include <stdio.h>
#include <wininet.h>
#pragma comment(lib, "WinInet.lib") 
#pragma warning( disable : 4554 ) // Я знаю как работают >> и <<
#define VAR_NAME(name)			#name

//FMT Version data
#define	VERSION					"v6.0"
#define VERSIONID				10U
//Icon Reaction IDs
#define IR_HELP					0U
#define IR_HIDESHOW				1U
#define IR_SYSSWITCH			2U
#define IR_EXIT					3U
#define IR_NEWVER				4U
//Logic Switch IDs
#define LID_SWAP				1U
#define LID_UPDOWN				2U
//Set Color IDs
#define SC_GREEN				FOREGROUND_GREEN
#define SC_SKY					FOREGROUND_BLUE | FOREGROUND_GREEN
#define SC_RED					FOREGROUND_RED
#define SC_YELLOW				FOREGROUND_GREEN | FOREGROUND_RED
#define SC_WHITE				FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
#define SC_NULL					SC_WHITE | BACKGROUND_RED
#define SetColor(colID)			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colID | FOREGROUND_INTENSITY)
//Time Sleep IDs
#define TS_PROCESSOR			10U
#define TS_SLOW_APP				250U
//Default struct data
#define D_STRDATA				{NULL, 0, 0}
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
//Animation Settings
#define ANIM_SIZE				60U
#define ANIM_INTERVAL			5U
//Timer Settings
#define TID_PROBLEM				1U
#define TID_CTRLC				2U
#define TID_CTRLV				3U
#define TID_INTRO				4U

typedef unsigned char			bool;

HWND _consoleWin =				NULL;
HWND _helpWin =					NULL;
HMODULE _histance =				NULL;

union BoolSettings
{
	bool memSell;
	struct
	{
		bool isExit				:	1;
		bool isVisible			:	1;
		bool isChangeLang		:	1;
		bool isNewVersion		:	1;
		bool isProblemKeys		:	1;
		bool isWaitingClipboard :	1;
		bool isWaitingInput		:	1;
	};
};
union BoolSettings _fmtSet = { 0, .isChangeLang = TRUE };
struct StrData
{
	wchar_t* str;
	size_t	size;
	unsigned format;
};
/////////////////////////////////////////////////////////////////////////////////////
const char* LoadAndWriteTXT(int nameID)
{
	const char* data = "";
	HINSTANCE handle = _histance;

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
	_consoleWin = FindMainWindow();
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
	case WM_TIMER:
		switch (commandID)
		{
		case TID_PROBLEM:
			if (GetKeyState(VK_LWIN) >= 0 && GetKeyState(VK_RWIN) >= 0
				&& GetKeyState(VK_MENU) >= 0 && GetKeyState(VK_CONTROL) >= 0)
					_fmtSet.isProblemKeys = FALSE;
			break;
		case TID_CTRLC:
			static unsigned char iterNULL = 0;
			if (++iterNULL == 15) //15 итераций проверки
			{
				_fmtSet.isWaitingClipboard = FALSE;
				iterNULL = 0;
			}
			break;
		case TID_CTRLV:
			for (unsigned char i = 0; i < 255; i++) 
				if (GetKeyState(i) < 0)
				{
					_fmtSet.isWaitingInput = FALSE;
					break;
				}
			break;
		default:
			break;
		}
		break;
	case WM_CLIPBOARDUPDATE:
		_fmtSet.isWaitingClipboard = FALSE;
		break;
	case WM_USER:
		//Создание менюшки с кнопками
		if (action == WM_RBUTTONUP || action == WM_LBUTTONUP || action == WM_MBUTTONDBLCLK)
		{
			HMENU hMenu = CreatePopupMenu();
			if (action == WM_MBUTTONDBLCLK)
			{
				AppendMenuW(hMenu, _fmtSet.isVisible ? MF_CHECKED : MF_UNCHECKED, IR_HIDESHOW,
					L"Режим разработчика");
				SetMenuItemBitmaps(hMenu, IR_HIDESHOW, MF_BYCOMMAND,
					LoadBitmapW(_histance, MAKEINTRESOURCEW(IDB_BITMAP3)), 0);
			}
			else
			{
				if (_fmtSet.isNewVersion)
				{
					AppendMenuW(hMenu, 0, IR_NEWVER, L"Доступна новая версия!");
					AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
				}
				AppendMenuW(hMenu, 0, IR_HELP, L"Помощь");
				AppendMenuW(hMenu, _fmtSet.isChangeLang ? MF_CHECKED : MF_UNCHECKED, IR_SYSSWITCH,
					L"Автоматически менять язык системы");
				AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenuW(hMenu, 0, IR_EXIT, L"Выход");

				SetMenuItemBitmaps(hMenu, IR_HELP, MF_BYCOMMAND,
					LoadBitmapW(_histance, MAKEINTRESOURCEW(IDB_BITMAP1)), 0);
				SetMenuItemBitmaps(hMenu, IR_SYSSWITCH, MF_BYCOMMAND,
					LoadBitmapW(_histance, MAKEINTRESOURCEW(IDB_BITMAP4)), 0);
				SetMenuItemBitmaps(hMenu, IR_EXIT, MF_BYCOMMAND,
					LoadBitmapW(_histance, MAKEINTRESOURCEW(IDB_BITMAP2)), 0);
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
			//Создание окна.
			if (!_helpWin)
				_helpWin = CreateWindowW(WCN_HELP, L"Помощь", WS_SYSMENU | WS_VISIBLE,
					(GetSystemMetrics(SM_CXSCREEN) >> 1) - (WH_WIDTH >> 1),
					(GetSystemMetrics(SM_CYSCREEN) >> 1) - (WH_HEIGHT >> 1),
					WH_WIDTH + WI_WIDTH, WH_HEIGHT + WI_HEIGHT, 0, 0, _histance, 0);
			else SetActiveWindow(_helpWin);
			break;
		case IR_HIDESHOW:
			_fmtSet.isVisible = !_fmtSet.isVisible;
			if (!_consoleWin)
			{
				AddDebugConsole();
				printf("%s\n", LoadAndWriteTXT(IDT_TEXT1));
			}
			else ShowWindow(_consoleWin, _fmtSet.isVisible ? SW_SHOW : SW_HIDE);
			break;
		case IR_SYSSWITCH:
			_fmtSet.isChangeLang = !_fmtSet.isChangeLang;
			break;
		case IR_EXIT:
			_fmtSet.isExit = TRUE;
			UnregisterHotKey(window, LID_SWAP);
			UnregisterHotKey(window, LID_UPDOWN);
			if(_helpWin)
				SendMessageW(_helpWin, WM_CLOSE, 0, 0);
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
	static unsigned frameID;
	switch (message)
	{
	case WM_CREATE:
		frameID = 0;
		hBitMap = (HBITMAP)LoadImageW(_histance,
			MAKEINTRESOURCEW(IDB_BITMAP5), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		SetTimer(window, TID_INTRO, ANIM_INTERVAL, NULL);
		return 0;
	case WM_TIMER:
		if (frameID != ANIM_SIZE - 1)
		{
			frameID++;
			InvalidateRect(window, NULL, TRUE);
		}
		else KillTimer(window, TID_INTRO);
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
		DestroyWindow(_helpWin);
		KillTimer(window, TID_INTRO);
		_helpWin = NULL;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(window, message, commandID, action);
}
/////////////////////////////////////////////////////////////////////////////////////
void LogMsg(const char* msg, WORD msgColor)
{
	if (!_consoleWin)
		return;
	SetColor(msgColor);
	printf("/#/ %s /#/\n", msg);
	SetColor(SC_WHITE);
}
void PrintStrData(struct StrData* data, const char* msg)
{
	if (!_consoleWin)
		return;
	SetColor(SC_RED);
	printf("%s[%llu] -> ", msg, data->size);
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
	struct StrData data = D_STRDATA;
	while (!OpenClipboard(0))
		Sleep(TS_PROCESSOR);
	LogMsg("Call GetBufer", SC_YELLOW);
	if (!IsClipboardFormatAvailable(data.format = CF_UNICODETEXT)
		&& !IsClipboardFormatAvailable(data.format = CF_DIB))
	{
		CloseClipboard();
		data.format = 0;
		return data;
	}
	wchar_t* clipData = (wchar_t*)GetClipboardData(data.format);
	data.size = wcslen(clipData) + 1;
	data.str = malloc(data.size * sizeof(wchar_t));
	if (data.str)
		memcpy(data.str, clipData, data.size * sizeof(wchar_t));
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
		HANDLE clipData = GlobalAlloc(GMEM_FIXED, data->size * sizeof(wchar_t));
		if (clipData)
			memcpy(clipData, data->str, data->size * sizeof(wchar_t));
		SetClipboardData(data->format, (HANDLE)clipData);
		free(data->str);
		data->str = NULL;
		data->size = 0;
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
		unsigned engCount;
		unsigned rusCount;
		wchar_t* oneWord;
		size_t wordLen;
		size_t lastID = 0;
		for (size_t i = 0; i < opt->size; i++)
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
	}
		break;
	case LID_UPDOWN:
	{
		wchar_t upChar;
		for (size_t i = 0; i < opt->size - 1; i++)
		{
			upChar = LUp(opt->str[i]);
			opt->str[i] = opt->str[i] == upChar ? LDown(opt->str[i]) : upChar;
		}
		LogMsg("Смена регистра выполнена", SC_GREEN);
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
	////	Проверка на существование запущенной программы
	HANDLE hMutex = CreateMutexW(0, TRUE, L"FMT");
	if (!hMutex || GetLastError() == ERROR_ALREADY_EXISTS)
		return 1;

	////	Поиск, создание и регестрация окна, проверки, и т.п.
	_histance = GetModuleHandleW(NULL);
	NOTIFYICONDATA icon;
	ZeroMemory(&icon, sizeof(icon));
	{
		////	Регистрация класса (штука, чтобы "CALLBACK IconReaction" вызывалась)
		WNDCLASS wc;	
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = IconReaction;
		wc.hInstance = _histance;
		wc.lpszClassName = WCN_ICON;
		RegisterClassW(&wc);

		////	Создание и добавление окна (в виде иконки) в трей
		icon.cbSize = sizeof(icon);
		icon.hWnd = CreateWindowW(wc.lpszClassName, wc.lpszClassName, 0, 0, 0, 0, 0, 0, 0, wc.hInstance, 0);
		icon.uVersion = NOTIFYICON_VERSION;
		icon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		icon.uCallbackMessage = WM_USER;
		//IDI_ICON1 - ID Иконки из файла ресурсов.
		icon.hIcon = LoadIconW(_histance, MAKEINTRESOURCEW(IDI_ICON1));
		const wchar_t* tip = L"Fix my text";
		wcscpy_s(icon.szTip, wcslen(tip) + 1, tip);

		////	Регистрация класса (штука, чтобы "CALLBACK HelpReaction" вызывалась)
		WNDCLASS wc2;
		ZeroMemory(&wc2, sizeof(wc2));
		wc2.lpfnWndProc = HelpReaction;
		wc2.hInstance = _histance;
		wc2.lpszClassName = WCN_HELP;
		wc2.hbrBackground = CreateSolidBrush(RGB(34, 30, 26));
		wc2.hIcon = LoadIconW(_histance, MAKEINTRESOURCEW(IDI_ICON1));
		RegisterClassW(&wc2);
	}
	Shell_NotifyIconW(NIM_ADD, &icon);

	////	ВСЕ ПЕРЕМЕННЫЕ 
	struct StrData conservation = D_STRDATA, selected = D_STRDATA;
	unsigned switcher = 0;

	////	Провкерка новой версии
	_fmtSet.isNewVersion = IsNewVersionExist();

	////	ЦАРЬ БАТЮШКА ЦИКЛ
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (!_fmtSet.isExit)
		if (GetMessageW(&msg, NULL, 0, 0))
		{
			DispatchMessageW(&msg);
			if (msg.message == WM_HOTKEY)
			{
				switcher = (unsigned)msg.wParam;
				if (_consoleWin)
					printf("--------------------------------------------------------------------------------------\n");

				////	Получение CONSERVATION
				conservation = GetBufer();
				PrintStrData(&conservation, VAR_NAME(conservation));

				////	Проблемные кнопки, мешающие копированию
				SetTimer(icon.hWnd, TID_PROBLEM, TS_PROCESSOR, NULL);
				_fmtSet.isProblemKeys = TRUE;
				while (_fmtSet.isProblemKeys)
					if (GetMessageW(&msg, NULL, 0, 0))
						DispatchMessageW(&msg);
				KillTimer(icon.hWnd, TID_PROBLEM);

				////	Эмуляция Ctrl+C
				AddClipboardFormatListener(icon.hWnd);
				SetTimer(icon.hWnd, TID_CTRLC, TS_PROCESSOR, NULL);
				_fmtSet.isWaitingClipboard = TRUE;
				EmulateACombinationWithCtrl('C');
				while (_fmtSet.isWaitingClipboard)
					if (GetMessageW(&msg, NULL, 0, 0))
						DispatchMessageW(&msg);
				KillTimer(icon.hWnd, TID_CTRLC);
				RemoveClipboardFormatListener(icon.hWnd);

				////	Получение SELECTED
				selected = GetBufer();
				PrintStrData(&selected, VAR_NAME(selected));

				////	Скипаем логику если ничего не выделенно
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
				LogicSwitch(&selected, switcher);														  
				PrintStrData(&selected, VAR_NAME(selected));
				SetBufer(&selected);

				////	Эмуляция Ctrl+V
				EmulateACombinationWithCtrl('V');

				////	Ожидания действий для корректной вставки CONSERVATION
				SetTimer(icon.hWnd, TID_CTRLV, TS_PROCESSOR, NULL);
				_fmtSet.isWaitingInput = TRUE;
				while (_fmtSet.isWaitingInput)
					if (GetMessageW(&msg, NULL, 0, 0))
						DispatchMessageW(&msg);
				KillTimer(icon.hWnd, TID_CTRLV);

				////	Вставка CONSERVATION
				SetBufer(&conservation);

				////	Смена языка (при включеной галочки)
				if (_fmtSet.isChangeLang && switcher == LID_SWAP)
					PostMessageW(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, 0, 0);
			}
		}
	////	Очитска
	DestroyWindow(icon.hWnd);
	Shell_NotifyIconW(NIM_DELETE, &icon);
	UnregisterClassW(WCN_ICON, _histance);
	UnregisterClassW(WCN_HELP, _histance);
	if (_consoleWin)
		FreeConsole();
	ReleaseMutex(hMutex);
	return 0;
}