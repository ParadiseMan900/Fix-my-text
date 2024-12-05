//Code for "Fix my text" by Teslev Dmitry Sergeevich 2023 - 2024
#include "resource.h"
#include <windows.h>
#include <locale.h>
#include <stdio.h>
#include <wininet.h>
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib, "WinInet.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma warning( disable : 4554 )	// Я знаю как работают >> <<
using namespace Gdiplus;
#define VAR_NAME(name)			#name

//FMT Version data
#define	VERSION					"v7.1"
#define VERSIONID				10U
//Icon Reaction IDs
#define IR_SETT					1U
#define IR_HIDESHOW				2U
#define IR_EXIT					3U
#define IR_NEWVER				4U
//Logic Switch IDs
#define LID_SWAP				1U
#define LID_UPDOWN				2U
#define LID_TOWEB				3U
#define LID_TRANSLATE			4U
//Set Color IDs
#define SC_GREEN				FOREGROUND_GREEN
#define SC_SKY					FOREGROUND_BLUE | FOREGROUND_GREEN
#define SC_RED					FOREGROUND_RED
#define SC_YELLOW				FOREGROUND_GREEN | FOREGROUND_RED
#define SC_WHITE				FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
#define SC_NULL					SC_WHITE | BACKGROUND_RED
#define ConsColor(colID)			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
									colID | FOREGROUND_INTENSITY)
//Сonstant case difference
#define CASE_DIFFERENCE			32U
//Window Class Name
#define WCN_ICON				L"FMT_TrayWin"
#define WCN_SETT				L"FMT_SettWin"
//Timer Settings
#define TID_PROBLEM				1U
#define TID_CTRLC				2U
#define TID_NEWVER				3U
//GitHub links
#define GITHUB_MAIN				L"https://github.com/ParadiseMan900/Fix-my-text"
#define GITHUB_RELEASE			L"https://github.com/ParadiseMan900/Fix-my-text/releases/latest"
//Settings name
#define SETT_FILE				L"%appdata%\\FMT_Settings.txt"
//Bool Settings Departments
#define BSID_CHANGE_LANG		0U
#define BSID_CHANGE_CAPS		1U
#define BSID_YANDEX_SEARCH		2U
#define BSID_YANDEX_TRANSLATE	3U
#define BSID_NEW_VERSION		0U
#define BSID_PROBLEM_KEYS		1U
#define BSID_WAITING_CLIPBOARD	2U
#define BSID_DEBUG_MODE			3U
#define BSID_DEBUG_VISIBLE		4U
#define BSID_PAGES_SPIN			5U
//Settings app values
#define	CX						1280U
#define CY						720U
#define MENU_BOX_COUNT_X        3U
#define MENU_BOX_COUNT_Y        12U
#define MENU_BTN_START_ID       2U
#define MOUSE_NULL_ID           -1
#define MOUSE_GH_ID             -2
#define COLR_CHOOSE_SIZE		3
#define KEY_NAME_SIZE			16
#define RECT_LTWH(rt)			(int)(rt).left, (int)(rt).top, (int)(rt).right - (int)(rt).left, (int)(rt).bottom - (int)(rt).top
#define ALLOC(memeory)			if(!(memeory)) exit(1)
//Page Blocks ID
#define PB_MASK                 0xFU
#define PBID_DESCRIPTION        0x0U
#define PBID_HOTKEY             0x1U
#define PBID_EXAMPLE            0x2U
#define PBID_CHECKBOX           0x3U
#define PBH_MASK                0xF0U
#define PBIDH_NOTHING           0x00U
#define PBIDH_STACK             0x10U
#define PBIDH_CUSTOM            0x20U
//Hotkey Buttons ID
#define HB_SIZE					3U
#define HBID_SAVE				0U
#define HBID_RESET				1U
#define HBID_CANCEL				2U

typedef struct _StrData
{
	wchar_t* str;
	size_t byteSize;
	size_t strSize;
	USHORT format;
} StrData;
typedef struct _Box_Colors
{
	COLORREF main;
	COLORREF bg;
} BoxColors;
typedef struct _Hotkey
{
	byte sys;
	byte main;
} Hotkey;
typedef struct _BlockHotkey
{
	Hotkey* hk;
	byte keysCount;
	wchar_t nameArr[5][KEY_NAME_SIZE];
	USHORT voidSize;
	int fontCy;
	RECT keysRt;
} BHOTKEY;
typedef struct _BlockChekbox
{
	bool* cBoxRef;
	void (*cBoxAction)();
	RECT cBoxRt;
} BCHECKBOX;
typedef void* BOBJECT;
typedef struct _Block
{
	RECT rt;
	byte style;
	//Зависящие от style
	const wchar_t* header;
	const wchar_t* text;
	BOBJECT obj;
} Block;
typedef struct _FuncPage
{
	const wchar_t* funcName;
	byte blocksCount;
	Block* blocks;
	RECT btnRt;
} FuncPage;
typedef struct _HotkeyVariables
{
	HHOOK hook;
	BHOTKEY userObj;
	Hotkey userHk;
	bool isFixed;
	bool isCanSet;
	RECT winRt;
	RECT btnArrRt[HB_SIZE];
	char blockID;
} KeyVar;
typedef struct _GraphicalVariables
{
	HWND settWin;
	FuncPage* pages;
	byte pagesCount;
	bool isNeedToSave, isActiveWin;
	HFONT font;
	USHORT voidSize, boxCx, boxCy;
	USHORT fontS, fontM, fontL, fontXL;
	USHORT boxOffset, textOffset, headerOffset, hotkeyOffset;
	char optID, mouseID, clickID;
	RECT githubRt;
	KeyVar* kv;
	byte lastDPI;
} GraphVar;

//	Глобальности
HMODULE histance = 0;
HWND consoleWin = 0;
NOTIFYICONDATAW nTray;
GraphVar* gv = 0;
Hotkey defaultHkSett[] = { { MOD_SHIFT | MOD_WIN, 'Z' }, { MOD_SHIFT | MOD_WIN, 'X' }, { MOD_ALT | MOD_WIN, 'Z' }, { MOD_ALT | MOD_WIN, 'X' } };
Hotkey fileHkSett[sizeof(defaultHkSett) / sizeof(*defaultHkSett)];
bool fileBoolSett[] = { 1, 1, 1, 1 };
bool tempSett[] = { 0, 0, 0, 0, 0, 0};

void LogMsg(const char* msg, WORD msgColor)
{
	if (!consoleWin)
		return;
	ConsColor(msgColor);
	printf("/#/ %s /#/\n", msg);
	ConsColor(SC_WHITE);
}
/////////////////////////////////////////////////////////////////////////////////////
void PrintStrData(StrData* data, const char* msg)
{
	if (!consoleWin)
		return;
	ConsColor(SC_RED);
	printf("%s[%llu] -> ", msg, data->strSize);
	switch (data->format)
	{
	case CF_UNICODETEXT:
		ConsColor(SC_WHITE);
		wprintf(L"\"%s\"\n", data->str);
		break;
	case CF_DIB:
		ConsColor(SC_SKY);
		printf("* PICTURE / КАРТИНКА *\n");
		ConsColor(SC_WHITE);
		break;
	default:
		ConsColor(SC_NULL);
		printf("|| NULL ||\n");
		ConsColor(SC_WHITE);
		break;
	}
}
StrData GetBufer(void)
{
	StrData data;
	ZeroMemory(&data, sizeof(StrData));
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
	data.str = (wchar_t*)malloc(data.byteSize);
	if (data.str)
		memcpy(data.str, clipData, data.byteSize);
	EmptyClipboard();
	CloseClipboard();
	return data;
}
void SetBufer(StrData* data)
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
		ZeroMemory(data, sizeof(StrData));
	}
	CloseClipboard();
	LogMsg("Call SetBufer", SC_YELLOW);
}
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
	return 0;
}
void LStrToInet(StrData* opt, const wchar_t* link)
{
	size_t linkSize = wcslen(link) + opt->strSize + 1;
	wchar_t* fullLink = (wchar_t*)calloc(linkSize, sizeof(wchar_t));
	if (fullLink)
	{
		wcscpy_s(fullLink, linkSize, link);
		wcscat_s(fullLink, linkSize, opt->str);
		ShellExecuteW(0, 0, fullLink, 0, 0, 0);
		free(fullLink);
	}
}
void LogicSwitch(StrData* opt, unsigned switcher)
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
		const wchar_t* tempP;
		const wchar_t* tempP2;
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
				tempP = tempP2 = engChars;
				(engCount >= rusCount ? tempP2 : tempP) = rusChars;
				for (size_t j = 0; j < wordLen; j++)
					for (size_t k = 0; k < swapCharsSize; k++)
						if (tempP[k] == oneWord[j])
						{
							oneWord[j] = tempP2[k];
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
		LStrToInet(opt, fileBoolSett[BSID_YANDEX_SEARCH] ? L"https://ya.ru?q=" : L"https://www.google.com/search?q=");
		LogMsg("Поиск по тексту выполнен", SC_GREEN);
		break;
	case LID_TRANSLATE:
		LStrToInet(opt, fileBoolSett[BSID_YANDEX_TRANSLATE] ? L"https://translate.yandex.ru/?source_lang=en&target_lang=ru&text=" :
			L"https://translate.google.ru/?sl=auto&tl=ru&text=");
		LogMsg("Перевод текеста выполнен", SC_GREEN);
		break;
	default:
		return;
	}
}
void EmulateCombination(const byte keysCount, ...)
{
	USHORT inputCount = keysCount * 2;
	INPUT* input = (INPUT*)calloc(inputCount, sizeof(INPUT));
	ALLOC(input);
	va_list valist;
	va_start(valist, keysCount);
	for (USHORT i = 0; i != keysCount; i++)
	{
		input[i].type = INPUT_KEYBOARD;
		input[i + keysCount].type = INPUT_KEYBOARD;
		input[i + keysCount].ki.dwFlags = KEYEVENTF_KEYUP;
		input[i].ki.wVk = input[i + keysCount].ki.wVk = va_arg(valist, WORD);
	}
	va_end(valist);
	if (SendInput(inputCount, input, sizeof(INPUT)) != inputCount)
		LogMsg("Emulate ERROR", SC_GREEN);
}
/////////////////////////////////////////////////////////////////////////////////////
void AdjustRectByDPI(RECT* rt, byte dpiNew, byte dpiOld)
{
	if (dpiNew == dpiOld)
		return;
	double ratio = (double)dpiNew / dpiOld;
	SIZE rtSize = { rt->right - rt->left, rt->bottom - rt->top };
	SIZE rtNewSize = { (long)(rtSize.cx * ratio), (long)(rtSize.cy * ratio) };
	rt->right = rt->left + rtNewSize.cx;
	rt->bottom = rt->top + rtNewSize.cy;
	OffsetRect(rt, -(rtNewSize.cx - rtSize.cx) >> 1, -(rtNewSize.cy - rtSize.cy) >> 1);
}
void AntiAdjustWindowRect(RECT* rt)
{
	RECT negativeRT = { 0, 0, 0, 0 };
	AdjustWindowRect(&negativeRT, WS_CAPTION, FALSE);	// +- {-8, -31, 8, 8}
	rt->left -= negativeRT.left;
	rt->top -= negativeRT.top;
	rt->right -= negativeRT.right;
	rt->bottom -= negativeRT.bottom;
}
HDC BeginPaintOnce(HWND window, LPPAINTSTRUCT lps, HBITMAP* hBmp)
{
	BeginPaint(window, lps);
	HDC hcdc = CreateCompatibleDC(lps->hdc);
	*hBmp = CreateCompatibleBitmap(lps->hdc, GetDeviceCaps(hcdc, HORZRES), GetDeviceCaps(hcdc, VERTRES));
	SelectObject(hcdc, *hBmp);
	return hcdc;
}
void EndPaintOnce(HWND window, LPPAINTSTRUCT lps, HBITMAP hBmp, HDC hcdc)
{
	BitBlt(lps->hdc, 0, 0, GetDeviceCaps(hcdc, HORZRES), GetDeviceCaps(hcdc, VERTRES), hcdc, 0, 0, SRCCOPY);
	DeleteDC(hcdc);
	DeleteObject(hBmp);
	EndPaint(window, lps);
}
void SetPage(const wchar_t* name, byte blocksCount, ...)
{
	FuncPage* curPage = (FuncPage*)realloc(gv->pages, ++gv->pagesCount * sizeof(FuncPage));
	if (!curPage)
		exit(1);
	gv->pages = curPage;
	curPage += gv->pagesCount - 1;
	ZeroMemory(curPage, sizeof(FuncPage));
	curPage->funcName = name;
	if (!(curPage->blocks = (Block*)calloc((curPage->blocksCount = blocksCount), sizeof(Block))))
		exit(1);
	va_list valist;
	va_start(valist, blocksCount);
	byte blockStyle;
	byte headStyle;
	for (size_t i = 0; i != blocksCount; i++)
	{
		curPage->blocks[i].style = va_arg(valist, byte);
		headStyle = curPage->blocks[i].style & PBH_MASK;
		blockStyle = curPage->blocks[i].style & PB_MASK;
		switch (headStyle)
		{
		case PBIDH_STACK:
		{
			const wchar_t* headers[] = { L"Описание", L"Сочетание", L"Примеры", L"Активация" };
			if (sizeof(headers) / sizeof(*headers) > blockStyle)
				curPage->blocks[i].header = headers[blockStyle];
			break;
		}
		case PBIDH_CUSTOM:
			curPage->blocks[i].header = va_arg(valist, wchar_t*);
			break;
		default:
			break;
		}
		switch (blockStyle)
		{
		case PBID_HOTKEY:
			curPage->blocks[i].text = L"Сочетание клавиш для активации";
			ALLOC(curPage->blocks[i].obj = malloc(sizeof(BHOTKEY)));
			((BHOTKEY*)curPage->blocks[i].obj)->hk = va_arg(valist, Hotkey*) - 1;
			break;
		case PBID_CHECKBOX:
			curPage->blocks[i].text = va_arg(valist, wchar_t*);
			ALLOC(curPage->blocks[i].obj = malloc(sizeof(BCHECKBOX)));
			*(BCHECKBOX*)curPage->blocks[i].obj = BCHECKBOX{ va_arg(valist, bool*), (void (*)())va_arg(valist, void*) };
			break;
		default:
			curPage->blocks[i].text = va_arg(valist, wchar_t*);
			break;
		}
	}
	va_end(valist);
}
void DeletePage(FuncPage* page)
{
	for (size_t i = 0; i != page->blocksCount; i++)
		if ((page->blocks[i].style & PB_MASK) == PBID_HOTKEY || (page->blocks[i].style & PB_MASK) == PBID_CHECKBOX)
			free(((page->blocks) + i)->obj);
	free(page->blocks);
}
int DrawInflateText(HDC hdc, const wchar_t* string, int size, RECT rect, UINT flags, int inflateW, int inflateH)
{
	if (flags & DT_CENTER)
		rect.right++;
	InflateRect(&rect, inflateW, inflateH);
	return DrawTextW(hdc, string, size, &rect, flags);
}
COLORREF BetweenColors(COLORREF start, COLORREF end, USHORT size, USHORT id)
{
	//  Формула для каждого канала: (y * id + x * (size - id - 1)) / (size - 1)
	COLORREF result = 0;
	for (byte i = 0; i != 3; i++)
		*((byte*)(&result) + i) = (*((byte*)(&end) + i) * id + *((byte*)(&start) + i) * (size - id - 1)) / (size - 1);
	return result;
}
void RedrawWinRect(RECT pageInfoRt)
{
	InflateRect(&pageInfoRt, 1, 1); // Небольшой отступ 
	InvalidateRect(gv->settWin, &pageInfoRt, TRUE);
}
void FillRoundRectangle(Graphics* g, Brush* b, int x, int y, int width, int height, unsigned radius)
{
	if (g == NULL)
		return;
	unsigned minVal = min(width, height);
	if (2 * radius > minVal)
		radius = minVal >> 1;
	GraphicsPath path;
	path.AddArc(x + width - 2 * radius, y, radius * 2, radius * 2, 270, 90);
	path.AddArc(x + width - 2 * radius, y + height - (int)radius * 2, radius * 2, radius * 2, 0, 90);
	path.AddArc(x, y + height - radius * 2, radius * 2, radius * 2, 90, 90);
	path.AddArc(x, y, radius * 2, radius * 2, 180, 90);

	path.CloseFigure();
	g->FillPath(b, &path);
}
Color ColorrefToColor(COLORREF colr)
{
	Color result;
	result.SetFromCOLORREF(colr);
	return result;
}
SIZE GetTextSize(HDC hdc, UINT format, const wchar_t* text, RECT rt)
{
	DrawTextW(hdc, text, -1, &rt, format | DT_CALCRECT);
	return { rt.right - rt.left, rt.bottom - rt.top };
}
HFONT SelectFont(HDC hdc, HFONT* hfont, int fontSize, int fontBold)
{
	LOGFONTW lfw;
	ZeroMemory(&lfw, sizeof(LOGFONTW));
	GetObjectW(*hfont, sizeof(LOGFONTW), &lfw);
	if (fontSize)
		lfw.lfHeight = fontSize;
	if (fontBold)
		lfw.lfWeight = fontBold;
	HFONT result = *hfont;
	*hfont = CreateFontIndirectW(&lfw);
	SelectObject(hdc, *hfont);
	return result;
}
void SelectFontOnce(HDC hdc, HFONT* hfont, int fontSize, int fontBold)
{
	DeleteObject(SelectFont(hdc, hfont, fontSize, fontBold));
}
void MovePageID(bool isDown)
{
	char tempID = gv->optID + (isDown ? 1 : -1);
	if (tempSett[BSID_PAGES_SPIN])
		tempID = (tempID + gv->pagesCount) % gv->pagesCount;
	else if (tempID == gv->pagesCount || tempID == -1)
		return;
	gv->optID = tempID;
	InvalidateRect(gv->settWin, 0, TRUE);
}
void ResetMouseID(void)
{
	gv->mouseID = MOUSE_NULL_ID;
	SetCursor((HCURSOR)LoadImageW(0, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
}
void GetNameByVK(UINT vk, wchar_t* buff)
{
	ZeroMemory(buff, KEY_NAME_SIZE);
	GetKeyNameTextW(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC) << 16, buff, KEY_NAME_SIZE - 1);
	if (!*buff)
		lstrcpyW(buff, L"NULL");
	else *buff = LUp(*buff);
}
SIZE CalculateHotkey(BHOTKEY* obj)
{
	HDC calcHdc = GetDC(gv->settWin);
	RECT clientRT;
	GetClientRect(gv->settWin, &clientRT);
	SIZE keysSize = { 0, obj->voidSize + obj->fontCy };
	HFONT saveFont = SelectFont(calcHdc, &gv->font, obj->fontCy, FW_BOLD);
	obj->keysCount = 0;
	const wchar_t* sysVk[] = { L"Alt", L"Ctrl", L"Shift", L"Win" };
	for (byte i = 0; i != sizeof(sysVk) / sizeof(*sysVk); i++)
		if (obj->hk->sys >> i & 1)
		{
			keysSize.cx += GetTextSize(calcHdc, 0, sysVk[i], clientRT).cx;
			lstrcpyW(obj->nameArr[obj->keysCount++], sysVk[i]);
		}
	if (obj->hk->main)
	{
		wchar_t buff[KEY_NAME_SIZE] = { 0 };
		GetNameByVK(obj->hk->main, buff);
		int mainKeyCx = GetTextSize(calcHdc, 0, buff, clientRT).cx;
		if (mainKeyCx < obj->fontCy)
			mainKeyCx = obj->fontCy;
		keysSize.cx += mainKeyCx;
		lstrcpyW(obj->nameArr[obj->keysCount++], buff);
	}
	if (!obj->keysCount)
	{
		wchar_t buff[] = L"Нажмите клавиши";
		keysSize.cx += GetTextSize(calcHdc, 0, buff, clientRT).cx + obj->voidSize;
		lstrcpyW(obj->nameArr[0], buff);
	}
	else keysSize.cx += obj->voidSize * obj->keysCount + gv->hotkeyOffset * (obj->keysCount - 1);
	DeleteObject(SelectObject(calcHdc, gv->font = saveFont));
	ReleaseDC(gv->settWin, calcHdc);
	return keysSize;
}
void CalculatePaintComponents(void)
{
	RECT clientRT;
	GetClientRect(gv->settWin, &clientRT);
	gv->voidSize = (USHORT)clientRT.bottom >> 5;
	gv->boxCx = ((USHORT)clientRT.right - (gv->voidSize * 3)) / MENU_BOX_COUNT_X;   //left = 1; right = 2; l + r = 3
	gv->boxCy = ((USHORT)clientRT.bottom - (gv->voidSize << 1)) / MENU_BOX_COUNT_Y;
	gv->fontS = gv->boxCy * 3 / 10;
	gv->fontM = gv->boxCy * 4 / 10;
	gv->fontL = gv->boxCy * 6 / 10;
	gv->fontXL = gv->boxCy * 8 / 10;
	gv->boxOffset = gv->boxCy - gv->fontM >> 1;
	gv->textOffset = gv->boxCy >> 1;
	gv->hotkeyOffset = gv->boxCy >> 3;
	gv->headerOffset = gv->voidSize >> 2;

	RECT pageRT;
	HDC calcHdc = GetDC(gv->settWin);
	SelectFontOnce(calcHdc, &gv->font, gv->fontM, FW_NORMAL);
	for (byte i = 0; i != gv->pagesCount; i++)
	{
		SetRect(&gv->pages[i].btnRt, gv->voidSize, gv->voidSize + gv->boxCy * (i + MENU_BTN_START_ID), gv->voidSize + gv->boxCx,
			gv->voidSize + gv->boxCy * (i + MENU_BTN_START_ID + 1));
		SetRect(&pageRT, 2 * gv->voidSize + gv->boxCx, gv->voidSize + 2 * gv->boxCy, clientRT.right - gv->voidSize, gv->voidSize + 2 * gv->boxCy);
		for (byte j = 0; j != gv->pages[i].blocksCount; j++)
		{
			if ((gv->pages[i].blocks[j].style & PBH_MASK) != PBIDH_NOTHING)
			{
				if (j)
					OffsetRect(&pageRT, 0, gv->voidSize);
				pageRT.bottom += gv->fontM + gv->headerOffset;
			}
			else if (j)
				OffsetRect(&pageRT, 0, gv->headerOffset);
			pageRT.bottom += GetTextSize(calcHdc, DT_WORDBREAK, gv->pages[i].blocks[j].text,
				{ pageRT.left + gv->textOffset, pageRT.top, pageRT.right - gv->textOffset, pageRT.bottom }).cy + gv->boxCy - gv->fontM;
			gv->pages[i].blocks[j].rt = pageRT;
			switch (gv->pages[i].blocks[j].style & PB_MASK)
			{
			case PBID_HOTKEY:
			{
				BHOTKEY* obj = (BHOTKEY*)gv->pages[i].blocks[j].obj;
				obj->fontCy = gv->fontM;
				obj->voidSize = gv->boxOffset;
				SIZE keysSize = CalculateHotkey(obj);
				SetRect(&obj->keysRt, pageRT.right - gv->textOffset - keysSize.cx, pageRT.bottom - ((int)gv->boxCy + keysSize.cy >> 1),
					pageRT.right - gv->textOffset, pageRT.bottom - ((int)gv->boxCy - keysSize.cy >> 1));
				break;
			}
			case PBID_CHECKBOX:
			{
				BCHECKBOX* obj = (BCHECKBOX*)gv->pages[i].blocks[j].obj;
				SetRect(&obj->cBoxRt, pageRT.right - gv->textOffset - gv->fontM * 2, pageRT.bottom - gv->boxOffset - gv->fontM,
					pageRT.right - gv->textOffset, pageRT.bottom - gv->boxOffset);
				break;
			}
			default:
				break;
			}
			pageRT.top = pageRT.bottom;
		}
	}
	ReleaseDC(gv->settWin, calcHdc);
	SetRect(&gv->githubRt, gv->voidSize, clientRT.bottom - (gv->voidSize + gv->boxCy), gv->voidSize + gv->boxCx,
		clientRT.bottom - gv->voidSize);
}
void DrawHotkey(HDC hdc, Graphics* gdiGraph, BHOTKEY* obj, RECT rt, COLORREF colrText, COLORREF colrBox)
{
	//  Вывод сочетания клавиш
	COLORREF saveTextColr = SetTextColor(hdc, colrText);
	HFONT saveFont = SelectFont(hdc, &gv->font, obj->fontCy, FW_BOLD);
	SolidBrush brush{ ColorrefToColor(colrBox) };
	rt.left += rt.right - rt.left - (obj->keysRt.right - obj->keysRt.left) >> 1;
	int rtOffset = (rt.bottom - rt.top) - (obj->keysRt.bottom - obj->keysRt.top) >> 1;
	rt.top += rtOffset;
	rt.bottom -= rtOffset;
	int keyCx = 0;
	bool isEmpty = !obj->keysCount;
	if (isEmpty)
		obj->keysCount++;
	for (unsigned char i = 0; i != obj->keysCount; i++)
	{
		keyCx = GetTextSize(hdc, 0, obj->nameArr[i], rt).cx;
		if (keyCx < obj->fontCy)
			keyCx = obj->fontCy;
		rt.right = rt.left + keyCx + obj->voidSize;
		FillRoundRectangle(gdiGraph, &brush, RECT_LTWH(rt), gv->boxCy >> 3);
		DrawInflateText(hdc, obj->nameArr[i], -1, rt, DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_CENTER, 0, 0);
		OffsetRect(&rt, keyCx + obj->voidSize + gv->hotkeyOffset, 0);
	}
	if (isEmpty)
		obj->keysCount--;
	SetTextColor(hdc, saveTextColr);
	DeleteObject(SelectObject(hdc, gv->font = saveFont));
}
void CloseHotkeyWin(void)
{
	UnhookWindowsHookEx(gv->kv->hook);
	ResetMouseID();
	free(gv->kv->userObj.hk);
	free(gv->kv);
	gv->kv = 0;
	InvalidateRect(gv->settWin, 0, TRUE);
}
void SetUserHotkey(Hotkey hk)
{
	gv->isNeedToSave = TRUE;
	BHOTKEY* obj = (BHOTKEY*)gv->pages[gv->optID].blocks[gv->kv->blockID].obj;
	UnregisterHotKey(nTray.hWnd, gv->optID + 1);
	*obj->hk = hk;
	RegisterHotKey(nTray.hWnd, gv->optID + 1, hk.sys, hk.main);
	SIZE hkSize = CalculateHotkey(obj);
	obj->keysRt.left = obj->keysRt.right - hkSize.cx;
	CloseHotkeyWin();
}
RECT* GetRtByBID(byte id)
{
	switch (gv->pages[gv->optID].blocks[id].style & PB_MASK)
	{
	case PBID_HOTKEY: return &((BHOTKEY*)gv->pages[gv->optID].blocks[id].obj)->keysRt;
	case PBID_CHECKBOX: return &((BCHECKBOX*)gv->pages[gv->optID].blocks[id].obj)->cBoxRt;
	default: break;
	}
	return 0;
}
RECT* GetRtByMouseID(char id)
{
	switch (id)
	{
	case MOUSE_NULL_ID:	return 0;																										break;
	case MOUSE_GH_ID: return &gv->githubRt;																								break;
	default: return !gv->kv ? (id < gv->pagesCount ? &gv->pages[id].btnRt : GetRtByBID(id - gv->pagesCount)) : &gv->kv->btnArrRt[id];	break;
	}
}
LRESULT CALLBACK LLKeyReaction(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		DWORD vk = ((LPKBDLLHOOKSTRUCT)lParam)->vkCode;
		Hotkey lastHk = gv->kv->userHk;
		LRESULT resultVal = gv->isActiveWin ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam);
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
		{
			if(!gv->isActiveWin)
				return resultVal;
			switch (vk)
			{
			case VK_ESCAPE:						CloseHotkeyWin();																return resultVal;
			case VK_LMENU: case VK_RMENU:       gv->kv->userHk.sys |= MOD_ALT;													break;
			case VK_LCONTROL: case VK_RCONTROL: gv->kv->userHk.sys |= MOD_CONTROL;												break;
			case VK_LSHIFT: case VK_RSHIFT:     gv->kv->userHk.sys |= MOD_SHIFT;												break;
			case VK_LWIN: case VK_RWIN:         gv->kv->userHk.sys |= MOD_WIN;													break;
			default:                            gv->kv->userHk.main = (byte)vk;													break;
			}
			if (lastHk.main == gv->kv->userHk.main && lastHk.sys == gv->kv->userHk.sys)
				return resultVal;
			gv->kv->isFixed = gv->kv->isCanSet = 0;
			if (gv->kv->userHk.main && gv->kv->userHk.sys)
			{
				gv->kv->isFixed = TRUE;
				wchar_t buff[KEY_NAME_SIZE] = { 0 };
				GetNameByVK(gv->kv->userHk.main, buff);
				if (gv->kv->isCanSet = (lstrcmpW(buff, L"NULL") && RegisterHotKey(nTray.hWnd, 0, gv->kv->userHk.sys, gv->kv->userHk.main)))
					UnregisterHotKey(nTray.hWnd, 0);
			}
		}
		else
		{
			switch (vk)
			{
			case VK_LMENU: case VK_RMENU:       gv->kv->userHk.sys &= ~MOD_ALT;													break;
			case VK_LCONTROL: case VK_RCONTROL: gv->kv->userHk.sys &= ~MOD_CONTROL;												break;
			case VK_LSHIFT: case VK_RSHIFT:     gv->kv->userHk.sys &= ~MOD_SHIFT;												break;
			case VK_LWIN: case VK_RWIN:         gv->kv->userHk.sys &= ~MOD_WIN;													break;
			default:							if (vk == gv->kv->userHk.main) gv->kv->userHk.main = 0; else return resultVal;	break;
			}
			if (gv->kv->isFixed)
				return resultVal;
		}
		*gv->kv->userObj.hk = gv->kv->userHk;
		SIZE size = CalculateHotkey(&gv->kv->userObj);
		gv->kv->userObj.keysRt = { 0, 0, size.cx, size.cy };
		RedrawWinRect(gv->kv->winRt);
		return resultVal;
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
LRESULT CALLBACK SettReaction(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		ALLOC(gv = (GraphVar*)calloc(1, sizeof(GraphVar)));
		gv->settWin = window;
		gv->isActiveWin = TRUE;
		gv->lastDPI = (byte)GetDpiForWindow(window);
		ResetMouseID();
		//		Устройство SetPage:
		//	1. Название страницы
		//	2. Количество блоков
		//	3. Стиль блока и стиль надписи
		//	4. Различные необходимые переменные для каждого стиля
		SetPage(L"Смена языковой раскладки", 5,
			PBID_DESCRIPTION | PBIDH_STACK, L"Функция позволяет изменить языковую раскладку выделенного текста",
			PBID_HOTKEY | PBIDH_STACK, fileHkSett + LID_SWAP,
			PBID_EXAMPLE | PBIDH_STACK, L"• Ghbdtn vbh! -> Привет мир!",
			PBID_EXAMPLE, L"• Иуфгешагд цуферукб шытэе ше, -> Beautiful weather, isn't it?",
			PBID_CHECKBOX | PBIDH_CUSTOM, L"Авто-смена", L"Автоматическия смена языка системы после использования", fileBoolSett + BSID_CHANGE_LANG, 0);
		SetPage(L"Смена буквенного регистра", 4,
			PBID_DESCRIPTION | PBIDH_STACK, L"Функция позволяет изменить буквенный регистр выделенного текста",
			PBID_HOTKEY | PBIDH_STACK, fileHkSett + LID_UPDOWN,
			PBID_EXAMPLE | PBIDH_STACK, L"• сЛУЧЙНЫЙ НАБОР СЛОВ -> Случйный набор слов",
			PBID_CHECKBOX | PBIDH_CUSTOM, L"Авто-смена", L"Автоматическия смена регистра системы после использования", fileBoolSett + BSID_CHANGE_CAPS, 0);
		SetPage(L"Поиск текста в браузере", 3,
			PBID_DESCRIPTION | PBIDH_STACK, L"Функция отпрвляет выделенный текст как запрос в браузерный поисковик",
			PBID_HOTKEY | PBIDH_STACK, fileHkSett + LID_TOWEB,
			PBID_CHECKBOX | PBIDH_CUSTOM, L"Поисковая система", L"Использовать Яндекс вместо Google", fileBoolSett + BSID_YANDEX_SEARCH, 0);
		SetPage(L"Открытие текста через переводчик", 3,
			PBID_DESCRIPTION | PBIDH_STACK, L"Функция отпрвляет выделенный текст как запрос в браузерный переводчик",
			PBID_HOTKEY | PBIDH_STACK, fileHkSett + LID_TRANSLATE,
			PBID_CHECKBOX | PBIDH_CUSTOM, L"Переводчик", L"Использовать Яндекс вместо Google", fileBoolSett + BSID_YANDEX_TRANSLATE, 0);
		gv->font = CreateFontW(0, 0, 0, 0, FW_NORMAL, 0, 0, 0, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			PROOF_QUALITY, DEFAULT_PITCH, L"Bahnschrift");
		RECT winRT;
		GetWindowRect(window, &winRT);
		AdjustRectByDPI(&winRT, gv->lastDPI, USER_DEFAULT_SCREEN_DPI);
		AdjustWindowRect(&winRT, WS_CAPTION, 0);
		SetWindowPos(window, 0, RECT_LTWH(winRT), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
		CalculatePaintComponents();
		SetForegroundWindow(window);
		return 0;
	case WM_NCCREATE:
		EnableNonClientDpiScaling(window);
		break;
	case WM_DPICHANGED:
	{
		byte dpi = (byte)LOWORD(wParam);
		RECT newWinRT;
		GetWindowRect(window, &newWinRT);
		AntiAdjustWindowRect(&newWinRT);
		AdjustRectByDPI(&newWinRT, dpi, gv->lastDPI);
		gv->lastDPI = dpi;
		AdjustWindowRect(&newWinRT, WS_CAPTION, 0);
		SetWindowPos(window, 0, RECT_LTWH(newWinRT), SWP_NOZORDER | SWP_NOACTIVATE);
		CalculatePaintComponents();
		InvalidateRect(gv->settWin, 0, TRUE);
		return 0;
	}
	case WM_PAINT:
	{
		RECT clientRT;
		GetClientRect(window, &clientRT);
		COLORREF colrWhite = RGB(255, 242, 229), colrBlack = RGB(34, 30, 26), colrGray = RGB(61, 53, 46),
			colrFunction = BetweenColors(RGB(248, 54, 1), RGB(249, 204, 34), gv->pagesCount, gv->optID);
		USHORT lineWidth = gv->boxCy >> 3;
		Pen pen{ ColorrefToColor(colrFunction), (REAL)lineWidth };
		pen.SetLineCap(LineCap::LineCapRound, LineCap::LineCapRound, DashCap::DashCapRound);
		SolidBrush brush{ ColorrefToColor(colrBlack) };
		RECT boxRT = { (long)gv->voidSize, (long)gv->voidSize, (long)gv->voidSize + (long)gv->boxCx, (long)gv->voidSize + (long)gv->boxCy };
		PAINTSTRUCT ps;
		HBITMAP hbitmap;

		HDC hdc = BeginPaintOnce(window, &ps, &hbitmap);
		Graphics gdiGraph(hdc);
		//  Тёмный задний фон
		gdiGraph.FillRectangle(&brush, 0, 0, clientRT.right, clientRT.bottom);
		brush.SetColor(ColorrefToColor(colrGray));
		gdiGraph.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

		//  ЛЕВАЯ ЧАСТЬ
		//  Заголовок левого меню
		SetTextColor(hdc, colrWhite);
		SetBkMode(hdc, TRANSPARENT);
		SelectFontOnce(hdc, &gv->font, gv->fontL, FW_BOLD);
		DrawInflateText(hdc, L"Все функции:", -1, boxRT, DT_VCENTER | DT_SINGLELINE | DT_NOCLIP, -gv->textOffset, 0);
		//  Отрисовка разделяющей линии 
		OffsetRect(&boxRT, 0, gv->boxCy);
		gdiGraph.DrawLine(&pen, (int)boxRT.left, (int)boxRT.top + boxRT.bottom >> 1, (int)boxRT.right,
			(int)boxRT.top + boxRT.bottom >> 1);
		//  Отрисовка копок функций
		SelectFontOnce(hdc, &gv->font, gv->fontM, FW_NORMAL);
		for (byte i = 0; i != gv->pagesCount; i++)
		{
			if (i == gv->mouseID || i == gv->optID)
			{
				FillRoundRectangle(&gdiGraph, &brush, RECT_LTWH(gv->pages[i].btnRt), gv->boxCy >> 2);
				if (i == gv->optID)
					gdiGraph.DrawLine(&pen, (int)gv->pages[i].btnRt.left + (lineWidth >> 1), (int)gv->pages[i].btnRt.top + (gv->boxCy >> 2),
						(int)gv->pages[i].btnRt.left + (lineWidth >> 1), (int)gv->pages[i].btnRt.bottom - (gv->boxCy >> 2));
			}
			DrawInflateText(hdc, gv->pages[i].funcName, -1, gv->pages[i].btnRt, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS, -gv->textOffset, 0);
		}
		//  Отрисовка разделяющей линии
		boxRT = gv->githubRt;
		OffsetRect(&boxRT, 0, -(int)gv->boxCy);
		gdiGraph.DrawLine(&pen, (int)boxRT.left, (int)boxRT.top + boxRT.bottom >> 1, (int)boxRT.right, (int)boxRT.top + boxRT.bottom >> 1);
		// Отрисовка копки гитхаба
		if (gv->mouseID == MOUSE_GH_ID)
			FillRoundRectangle(&gdiGraph, &brush, RECT_LTWH(gv->githubRt), gv->boxCy >> 2);
		DrawInflateText(hdc, L"Страница приложения", -1, gv->githubRt, DT_VCENTER | DT_SINGLELINE | DT_NOCLIP, -gv->textOffset, 0);

		//  ПРАВАЯ ЧАСТЬ
		//  Заголовок выбранной функции
		SelectFontOnce(hdc, &gv->font, gv->fontXL, FW_BOLD);
		SetRect(&boxRT, 2 * gv->voidSize + gv->boxCx, gv->voidSize, clientRT.right - gv->voidSize, gv->voidSize + gv->boxCy);
		DrawInflateText(hdc, gv->pages[gv->optID].funcName, -1, boxRT, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS, 0, 0);
		//  Отрисовка всех блоков страницы (заданных функцией SetPage)
		SelectFontOnce(hdc, &gv->font, gv->fontM, FW_NORMAL);
		for (byte i = 0; i != gv->pages[gv->optID].blocksCount; i++)
		{
			boxRT = gv->pages[gv->optID].blocks[i].rt;
			//  Заголовок блока
			if ((gv->pages[gv->optID].blocks[i].style & PBH_MASK) != PBIDH_NOTHING)
			{
				SelectFontOnce(hdc, &gv->font, 0, FW_BOLD);
				boxRT.top += gv->headerOffset + DrawInflateText(hdc, gv->pages[gv->optID].blocks[i].header, -1, boxRT,
					DT_WORDBREAK | DT_SINGLELINE | DT_WORD_ELLIPSIS, 0, 0);
				SelectFontOnce(hdc, &gv->font, 0, FW_NORMAL);
			}
			//  Блок + текст
			FillRoundRectangle(&gdiGraph, &brush, RECT_LTWH(boxRT), gv->boxCy >> 3);
			DrawInflateText(hdc, gv->pages[gv->optID].blocks[i].text, -1, boxRT, DT_WORDBREAK, -gv->textOffset, -gv->boxOffset);
			//  Различные реализации при разных стилях
			switch (gv->pages[gv->optID].blocks[i].style & PB_MASK)
			{
			case PBID_HOTKEY:
			{
				//	Отрисовка хоткея
				DrawHotkey(hdc, &gdiGraph, (BHOTKEY*)gv->pages[gv->optID].blocks[i].obj, ((BHOTKEY*)gv->pages[gv->optID].blocks[i].obj)->keysRt,
					colrBlack, BetweenColors(colrFunction, colrWhite, COLR_CHOOSE_SIZE, i + gv->pagesCount == gv->mouseID));
				break;
			}
			case PBID_CHECKBOX:
			{
				BCHECKBOX* obj = (BCHECKBOX*)gv->pages[gv->optID].blocks[i].obj;
				//  Получение булевого значения
				bool isOn = obj->cBoxRef && *obj->cBoxRef;
				BoxColors colrOnOff = { colrBlack, colrBlack };
				(isOn ? colrOnOff.bg : colrOnOff.main) = colrFunction;
				//  Отрисовка переключателя 
				boxRT = obj->cBoxRt;
				int cBoxCy = boxRT.bottom - boxRT.top;
				brush.SetColor(ColorrefToColor(BetweenColors(colrOnOff.bg, colrWhite, COLR_CHOOSE_SIZE, i + gv->pagesCount == gv->mouseID)));
				FillRoundRectangle(&gdiGraph, &brush, RECT_LTWH(boxRT), cBoxCy);
				isOn ? (boxRT.left += cBoxCy) : (boxRT.right -= cBoxCy);
				InflateRect(&boxRT, -(cBoxCy >> 2), -(cBoxCy >> 2));
				brush.SetColor(ColorrefToColor(colrOnOff.main));
				gdiGraph.FillEllipse(&brush, RECT_LTWH(boxRT));
				brush.SetColor(ColorrefToColor(colrGray));
				break;
			}
			default: break;
			}
		}
		//	Если выбор пользовательского хоткея
		if (gv->kv)
		{
			RECT hkBoxRt = gv->kv->winRt;
			//	Затемнение всего окна
			brush.SetColor(Color{ 127, 0, 0, 0 });
			gdiGraph.SetSmoothingMode(Gdiplus::SmoothingModeNone);
			gdiGraph.FillRectangle(&brush, 0, 0, clientRT.right, clientRT.bottom);
			gdiGraph.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			//	Рисование окна хоткея
			brush.SetColor(ColorrefToColor(colrGray));
			FillRoundRectangle(&gdiGraph, &brush, RECT_LTWH(gv->kv->winRt), gv->boxCy >> 1);
			//	Отрисовка заголовка
			int partCy = (gv->kv->winRt.bottom - gv->kv->winRt.top) / 4;
			SelectFontOnce(hdc, &gv->font, gv->fontL, FW_BOLD);
			SetRect(&boxRT, gv->kv->winRt.left, gv->kv->winRt.top + gv->voidSize, gv->kv->winRt.right, gv->kv->winRt.top + gv->voidSize + gv->fontL);
			DrawInflateText(hdc, L"Сочетание клавиш активации", -1, boxRT, DT_SINGLELINE | DT_NOCLIP, -gv->textOffset, 0);
			//	Отрисовка подзаголовка
			SelectFontOnce(hdc, &gv->font, gv->fontM, FW_NORMAL);
			SetRect(&boxRT, boxRT.left, boxRT.bottom, boxRT.right, boxRT.bottom + gv->fontM);
			DrawInflateText(hdc, L"Введите сочетание клавиш для изменения:", -1, boxRT, DT_SINGLELINE | DT_NOCLIP, -gv->textOffset, 0);
			hkBoxRt.top = boxRT.bottom;
			//	Отрисовка примичания
			const wchar_t postScript[] = L"Допустимы только сочетания клавиш, имеющие не менее одной модальной клавиши, такие как  Alt, Ctrl, Shift или Win.";
			SelectFontOnce(hdc, &gv->font, gv->fontS, 0);
			USHORT postScriptCy = (USHORT)GetTextSize(hdc, DT_WORDBREAK, postScript, { gv->kv->winRt.left + gv->textOffset, gv->kv->winRt.top,
				gv->kv->winRt.right - gv->textOffset,  gv->kv->winRt.bottom }).cy;
			SetRect(&boxRT, boxRT.left, gv->kv->btnArrRt->top - gv->voidSize - postScriptCy, boxRT.right, gv->kv->btnArrRt->top - gv->voidSize);
			DrawInflateText(hdc, postScript, -1, boxRT, DT_WORDBREAK, -gv->textOffset, 0);
			hkBoxRt.bottom = boxRT.top;
			//	Отрисовка хоткея
			BoxColors hotkeyColr = (gv->kv->isFixed ? (gv->kv->isCanSet ? BoxColors{ colrBlack, colrFunction } : BoxColors{ colrFunction, colrBlack }) :
				(gv->kv->userObj.keysCount ? BoxColors{ colrBlack, colrWhite } : BoxColors{ colrWhite, colrBlack }));
			DrawHotkey(hdc, &gdiGraph, &gv->kv->userObj, hkBoxRt, hotkeyColr.main, hotkeyColr.bg);
			//	Вывод кнопок
			const wchar_t* bntNames[HB_SIZE] = { L"Сохранить", L"Сброс", L"Отмена" };
			SelectFontOnce(hdc, &gv->font, gv->fontM, FW_BOLD);
			hotkeyColr = gv->kv->isCanSet ? BoxColors{ colrBlack, BetweenColors(colrFunction, colrWhite, COLR_CHOOSE_SIZE, !gv->mouseID) } :
				BoxColors{ colrGray, colrBlack };
			SetTextColor(hdc, hotkeyColr.main);
			brush.SetColor(ColorrefToColor(hotkeyColr.bg));
			FillRoundRectangle(&gdiGraph, &brush, RECT_LTWH(gv->kv->btnArrRt[0]), gv->boxCy >> 2);
			DrawInflateText(hdc, bntNames[0], -1, gv->kv->btnArrRt[0], DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_CENTER, 0, 0);
			SetTextColor(hdc, colrWhite);
			for (byte i = 1; i != HB_SIZE; i++)
			{
				brush.SetColor(ColorrefToColor(BetweenColors(colrBlack, colrWhite, COLR_CHOOSE_SIZE, i == gv->mouseID)));
				FillRoundRectangle(&gdiGraph, &brush, RECT_LTWH(gv->kv->btnArrRt[i]), gv->boxCy >> 2);
				DrawInflateText(hdc, bntNames[i], -1, gv->kv->btnArrRt[i], DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_CENTER, 0, 0);
			}
		}
		EndPaintOnce(window, &ps, hbitmap, hdc);
		return 0;
	}
	case WM_MOUSEWHEEL:
		switch (LOWORD(wParam))
		{
		case MK_CONTROL:
		{
			RECT winRT;
			GetWindowRect(window, &winRT);
			char mouseDirection = (short)HIWORD(wParam) >= 0 ? 1 : -1;
			SIZE infVal = { GetSystemMetrics(SM_CXSCREEN) >> 5, GetSystemMetrics(SM_CYSCREEN) >> 5 };
			if ((mouseDirection == 1 && (winRT.right - winRT.left + infVal.cx < GetSystemMetrics(SM_CXSCREEN) &&
				winRT.bottom - winRT.top + infVal.cy < GetSystemMetrics(SM_CYSCREEN))) ||
				(mouseDirection == -1 && (winRT.right - winRT.left - infVal.cx >= (GetSystemMetrics(SM_CXSCREEN) >> 1) &&
					winRT.bottom - winRT.top - infVal.cy >= (GetSystemMetrics(SM_CYSCREEN) >> 1))))
			{
				InflateRect(&winRT, infVal.cx * mouseDirection, infVal.cy * mouseDirection);
				SetWindowPos(window, 0, 0, 0, winRT.right - winRT.left, winRT.bottom - winRT.top,
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOMOVE);
				CalculatePaintComponents();
			}
			break;
		}
		case NULL:
		{
			if (!gv->kv)
				MovePageID((short)HIWORD(wParam) < 0);
			break;
		}
		default:
			break;
		}
		return 0;
	case WM_LBUTTONDOWN:
		gv->clickID = gv->mouseID;
		return 0;
	case WM_LBUTTONUP:
	{
		if (gv->clickID != gv->mouseID)
			return 0;
		POINT mousePt;
		GetCursorPos(&mousePt);
		ScreenToClient(window, &mousePt);
		if (!gv->kv)
			switch (gv->mouseID)
			{
			case MOUSE_NULL_ID: break;
			case MOUSE_GH_ID:	ShellExecuteW(window, 0, GITHUB_MAIN, 0, 0, 0); break;
			default:
				if (gv->mouseID < gv->pagesCount)
				{
					gv->optID = gv->mouseID;
					SetCursor((HCURSOR)LoadImageW(0, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
					InvalidateRect(window, 0, TRUE);
				}
				else
				{
					byte blockID = gv->mouseID - gv->pagesCount;
					switch (gv->pages[gv->optID].blocks[blockID].style & PB_MASK)
					{
					case PBID_HOTKEY:
					{
						BHOTKEY* obj = (BHOTKEY*)gv->pages[gv->optID].blocks[blockID].obj;
						if (PtInRect(&obj->keysRt, mousePt))
						{
							ResetMouseID();
							ALLOC(gv->kv = (KeyVar*)calloc(1, sizeof(KeyVar)));
							gv->kv->hook = SetWindowsHookExW(WH_KEYBOARD_LL, LLKeyReaction, 0, 0);
							gv->kv->isFixed = TRUE;
							gv->kv->blockID = blockID;
							ALLOC(gv->kv->userObj.hk = (Hotkey*)malloc(sizeof(Hotkey)));
							*gv->kv->userObj.hk = *obj->hk;
							gv->kv->userObj.fontCy = gv->fontL;
							gv->kv->userObj.voidSize = gv->boxOffset << 1;
							SIZE hkSize = CalculateHotkey(&gv->kv->userObj);
							SetRect(&gv->kv->userObj.keysRt, 0, 0, hkSize.cx, hkSize.cy);
							GetClientRect(gv->settWin, &gv->kv->winRt);
							InflateRect(&gv->kv->winRt, -gv->kv->winRt.bottom >> 1, -(gv->kv->winRt.bottom >> 3) - gv->voidSize);
							USHORT btnPartCx = (USHORT)((gv->kv->winRt.right - gv->kv->winRt.left) - (gv->voidSize + gv->hotkeyOffset) * 2) / 3;
							SetRect(gv->kv->btnArrRt, gv->kv->winRt.left + gv->voidSize, gv->kv->winRt.bottom - gv->voidSize - gv->boxCy,
								gv->kv->winRt.left + gv->voidSize + btnPartCx, gv->kv->winRt.bottom - gv->voidSize);
							for (byte i = 1; i != HB_SIZE; i++)
							{
								gv->kv->btnArrRt[i] = gv->kv->btnArrRt[i - 1];
								OffsetRect(gv->kv->btnArrRt + i, gv->hotkeyOffset + btnPartCx, 0);
							}
							InvalidateRect(gv->settWin, 0, TRUE);
						}
						break;
					}
					case PBID_CHECKBOX:
					{
						BCHECKBOX* obj = (BCHECKBOX*)gv->pages[gv->optID].blocks[blockID].obj;
						if (obj->cBoxRef && PtInRect(&obj->cBoxRt, mousePt))
						{
							gv->isNeedToSave = TRUE;
							*obj->cBoxRef = !*obj->cBoxRef;
							if (obj->cBoxAction)
								obj->cBoxAction();
							RedrawWinRect(obj->cBoxRt);
						}
						break;
					}
					default: break;
					}
				}
				break;
			}
		else
			switch (gv->mouseID)
			{
			case HBID_SAVE:		if (gv->kv->isCanSet)	SetUserHotkey(*gv->kv->userObj.hk);	break;
			case HBID_RESET:	SetUserHotkey(defaultHkSett[gv->optID]);					break;
			case HBID_CANCEL:	CloseHotkeyWin();											break;
			default:																		break;
			}
		return 0;
	}
	case WM_ACTIVATE:
		if (gv->kv)
			gv->isActiveWin = LOWORD(wParam);
		[[fallthrough]];
	case WM_SETCURSOR:
	{
		POINT mousePt;
		GetCursorPos(&mousePt);
		ScreenToClient(window, &mousePt);
		char saveChoiseID = gv->mouseID;
		gv->mouseID = MOUSE_NULL_ID;
		char startID = MOUSE_GH_ID, endID = gv->pagesCount + (char)gv->pages[gv->optID].blocksCount;
		if (gv->kv)
		{
			startID = 0;
			endID = HB_SIZE;
		}
		for (char i = startID; i != endID; i++)
			if (i != MOUSE_NULL_ID && PtInRect(GetRtByMouseID(i), mousePt))
			{
				gv->mouseID = i;
				break;
			}
		SetCursor((HCURSOR)LoadImageW(0, gv->mouseID == MOUSE_NULL_ID || gv->mouseID == gv->optID ? IDC_ARROW : IDC_HAND,
			IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
		if (gv->mouseID != saveChoiseID)
		{
			if (gv->mouseID != MOUSE_NULL_ID)
				RedrawWinRect(*GetRtByMouseID(gv->mouseID));
			if (saveChoiseID != MOUSE_NULL_ID)
				RedrawWinRect(*GetRtByMouseID(saveChoiseID));
		}
		return 0;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_UP || wParam == VK_DOWN)
			MovePageID(wParam - VK_UP);
		return 0;
	}
	case WM_CLOSE:
		// Экспорт настроек
		LogMsg("Сохранение настроек", SC_GREEN);
		if (gv->isNeedToSave)
		{
			wchar_t appdata_path[MAX_PATH];
			ExpandEnvironmentStringsW(SETT_FILE, appdata_path, MAX_PATH);
			SetFileAttributesW(appdata_path, FILE_ATTRIBUTE_NORMAL);
			FILE* fSettings;
			_wfopen_s(&fSettings, appdata_path, L"w");
			if (fSettings)
			{
				unsigned long long fileVal = 0;
				for (byte i = 0; i != sizeof(fileHkSett) / sizeof(*fileHkSett); i++)
				{
					fileVal = fileHkSett[i].main | ((unsigned long long)fileHkSett[i].sys << 8);
					fprintf_s(fSettings, "%llu ", fileVal);
				}
				fprintf_s(fSettings, "\n");
				fileVal = 0;
				for (byte i = 0; i != sizeof(fileBoolSett) / sizeof(*fileBoolSett); i++)
					fileVal |= ((unsigned long long)fileBoolSett[i]) << i;
				fprintf_s(fSettings, "%llu", fileVal);
				fclose(fSettings);
				SetFileAttributesW(appdata_path, FILE_ATTRIBUTE_READONLY);
			}
		}
		// Очистка кучи
		if (gv->kv)
			CloseHotkeyWin();
		DestroyWindow(window);
		for (byte i = 0; i != gv->pagesCount; i++)
			DeletePage(gv->pages + i);
		free(gv->pages);
		DeleteObject(gv->font);
		free(gv);
		gv = 0;
		return 0;
	}
	return DefWindowProcW(window, message, wParam, lParam);
}
/////////////////////////////////////////////////////////////////////////////////////
void SetBoolByAttribute(const wchar_t* cmdLine, const wchar_t* attribute, byte boolID)
{
	if (!lstrcmpW(cmdLine, attribute))
		tempSett[boolID] = TRUE;
}
bool IsNewVersionExist(void)
{
	bool result = 0;
	HINTERNET hInterOpen = InternetOpenW(L"fmt", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hInterOpen)
		return result;
	HINTERNET hInterconnect = InternetOpenUrlW(hInterOpen,
		L"https://api.github.com/repos/ParadiseMan900/Fix-my-text/tags?per_page=1", NULL, 0, 0, 0);
	if (!hInterconnect)
		return result;
	DWORD dataSize = 0;
	InternetQueryDataAvailable(hInterconnect, &dataSize, 0, 0);
	char* str = (char*)calloc((size_t)dataSize + 1, sizeof(char));
	if (!str)
		return result;
	InternetReadFile(hInterconnect, str, dataSize, &dataSize);
	InternetCloseHandle(hInterconnect);
	InternetCloseHandle(hInterOpen);
	if (*str == '{')
		return result;
	size_t verSize = 0;
	while (str[VERSIONID + verSize] != '\"')
		verSize++;
	char* version = (char*)calloc(verSize + 1, sizeof(char));
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
			data = (char*)LockResource(rcData);
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
	HMENU hMenu = GetSystemMenu(consoleWin, 0);
	if (hMenu)
		DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
}
void OpenSettings(void)
{
	if (!gv)
		CreateWindowExW(0, WCN_SETT, L"Настройки FMT", WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX,
			GetSystemMetrics(SM_CXSCREEN) - CX >> 1, GetSystemMetrics(SM_CYSCREEN) - CY >> 1, CX, CY, 0, 0, histance, 0);
	else SetForegroundWindow(gv->settWin);
}
void TimerWaitCycle(MSG* msg, byte timerID, byte boolID)
{
	SetTimer(nTray.hWnd, timerID, USER_TIMER_MINIMUM, NULL);
	tempSett[boolID] = TRUE;
	while (tempSett[boolID])
		if (GetMessageW(msg, NULL, 0, 0))
			DispatchMessageW(msg);
	KillTimer(nTray.hWnd, timerID);
}
void AddMenuButton(HMENU hMenu, UINT flags, byte id, const wchar_t* name, byte bmpID)
{
	AppendMenuW(hMenu, flags, id, name);
	if(bmpID)
		SetMenuItemBitmaps(hMenu, id, MF_BYCOMMAND,	(HBITMAP)LoadImageW(histance, MAKEINTRESOURCEW(bmpID), 0, 0, 0, LR_SHARED), 0);
}
LRESULT CALLBACK IconReaction(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		for (byte i = 0; i != sizeof(fileHkSett) / sizeof(*fileHkSett); i++)
			RegisterHotKey(window, i + 1, fileHkSett[i].sys, fileHkSett[i].main);
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case TID_PROBLEM:
			for (byte i = 0; i != 255; i++)
				if (GetKeyState(i) < 0)
					return 0;
			tempSett[BSID_PROBLEM_KEYS] = 0;
			break;
		case TID_CTRLC:
		{
			static unsigned char iterNULL = 0;
			if (++iterNULL == 25)	//25 итераций проверки
			{
				tempSett[BSID_WAITING_CLIPBOARD] = 0;
				iterNULL = 0;
			}
			break;
		}
		case TID_NEWVER:
			if (tempSett[BSID_NEW_VERSION] = IsNewVersionExist())
				KillTimer(window, TID_NEWVER);
			break;
		default:
			break;
		}
		return 0;
	case WM_CLIPBOARDUPDATE:
		tempSett[BSID_WAITING_CLIPBOARD] = 0;
		break;
	case WM_USER:
		switch (lParam)
		{
		case WM_RBUTTONUP:
		{
			//Создание менюшки с кнопками
			HMENU hMenu = CreatePopupMenu();
			if (tempSett[BSID_NEW_VERSION])
			{
				AddMenuButton(hMenu, 0, IR_NEWVER, L"Доступна новая версия!", 0);
				AddMenuButton(hMenu, MF_SEPARATOR, 0, 0, 0);
			}
			AddMenuButton(hMenu, 0, IR_SETT, L"Настройки", IDB_BITMAP1);
			AddMenuButton(hMenu, MF_SEPARATOR, 0, 0, 0);
			if (tempSett[BSID_DEBUG_MODE])
			{
				AddMenuButton(hMenu, tempSett[BSID_DEBUG_VISIBLE] ? MF_CHECKED : MF_UNCHECKED, IR_HIDESHOW,	L"Режим отладки", IDB_BITMAP3);
				AddMenuButton(hMenu, MF_SEPARATOR, 0, 0, 0);
			}
			AddMenuButton(hMenu, 0, IR_EXIT, L"Выход", IDB_BITMAP2);
			//Вывод меню
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(window);
			TrackPopupMenuEx(hMenu, TPM_BOTTOMALIGN | TPM_CENTERALIGN, pt.x, pt.y, window, 0);
			DestroyMenu(hMenu);
			break;
		}
		case WM_LBUTTONUP:
			OpenSettings();
			break;
		case WM_MBUTTONDBLCLK:
			tempSett[BSID_DEBUG_MODE] = TRUE;
			break;
		case NIN_BALLOONUSERCLICK:
			ShellExecuteW(window, 0, GITHUB_RELEASE, 0, 0, 0);
			break;
		default:
			break;
		}
		break;
	case WM_COMMAND:
		//Реакция на кнопки в меню
		switch (wParam)
		{
		case IR_SETT: OpenSettings(); break;
		case IR_HIDESHOW:
			tempSett[BSID_DEBUG_VISIBLE] = !tempSett[BSID_DEBUG_VISIBLE];
			if (!consoleWin)
			{
				AddDebugConsole();
				printf("%s\n", LoadAndWriteTXT(IDT_TEXT1));
			}
			else ShowWindow(consoleWin, tempSett[BSID_DEBUG_VISIBLE] ? SW_SHOW : SW_HIDE);
			break;
		case IR_EXIT:
			for (byte i = 0; i != sizeof(fileHkSett) / sizeof(*fileHkSett); i++)
				UnregisterHotKey(window, i + 1);
			if (gv)
				SendMessageW(gv->settWin, WM_CLOSE, 0, 0);
			PostQuitMessage(0);
			break;
		case IR_NEWVER:
			ShellExecuteW(window, 0, GITHUB_RELEASE, 0, 0, 0);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return DefWindowProcW(window, message, wParam, lParam);
}
/////////////////////////////////////////////////////////////////////////////////////
int WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hInstPrev, _In_ WCHAR* cmdLine, _In_ int cmdShow)
{
	////	Проверка на существование запущенной программы
	HANDLE hMutex = CreateMutexW(0, TRUE, L"FMT");
	if (!hMutex || GetLastError() == ERROR_ALREADY_EXISTS)
		return 1;
	//// Подключение GDI+
	ULONG_PTR token;
	{
		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&token, &gdiplusStartupInput, 0);
	}
	//// Импорт настроек
	{
		memcpy(fileHkSett, defaultHkSett, sizeof(fileHkSett));
		wchar_t appdata_path[MAX_PATH];
		ExpandEnvironmentStringsW(SETT_FILE, appdata_path, MAX_PATH);	//%appdata% -> Полный путь
		SetFileAttributesW(appdata_path, FILE_ATTRIBUTE_NORMAL);
		FILE* fSettings;
		_wfopen_s(&fSettings, appdata_path, L"r");
		if (fSettings)
		{
			int scanCode;
			unsigned long long fileVal;
			for (byte i = 0; i != sizeof(fileHkSett) / sizeof(*fileHkSett); i++)
			{
				scanCode = fscanf_s(fSettings, "%llu", &fileVal);
				if (scanCode && scanCode == EOF)
					break;
				fileHkSett[i].main = fileVal & 0xFF;
				fileHkSett[i].sys = (fileVal >> 8) & 0xFF;
			}
			scanCode = fscanf_s(fSettings, "%llu", &fileVal);
			if (scanCode && scanCode != EOF)
				for (byte i = 0; i != sizeof(fileBoolSett) / sizeof(*fileBoolSett); i++)
					fileBoolSett[i] = (fileVal >> i) & 1;
			fclose(fSettings);
			SetFileAttributesW(appdata_path, FILE_ATTRIBUTE_READONLY);
		}
	}
	////	Реакция на атрибуты
	{
		int argsCount;
		wchar_t** cmdArgvLine = CommandLineToArgvW(cmdLine, &argsCount);
		if (cmdArgvLine)
			for (int i = 0; i != argsCount; i++)
			{
				SetBoolByAttribute(cmdArgvLine[i], L"debug", BSID_DEBUG_MODE);
				SetBoolByAttribute(cmdArgvLine[i], L"spin", BSID_PAGES_SPIN);
			}
	}
	////	Поиск, создание и регестрация окна, настройки
	histance = hInst;
	{
		////	Регистрация классов
		WNDCLASS wc;
		WNDCLASSEX wc2;
		ZeroMemory(&wc, sizeof(wc));
		ZeroMemory(&wc2, sizeof(wc2));
		wc.lpfnWndProc = IconReaction;
		wc.hInstance = histance;
		wc.lpszClassName = WCN_ICON;
		wc2.cbSize = sizeof(wc2);
		wc2.lpfnWndProc = SettReaction;
		wc2.hInstance = histance;
		wc2.lpszClassName = WCN_SETT;
		wc2.hIcon = (HICON)LoadImageW(histance, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 256, 256, 0);
		wc2.hIconSm = (HICON)LoadImageW(histance, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
		RegisterClassW(&wc);
		RegisterClassExW(&wc2);
		////	Создание и добавление окна (в виде иконки) в трей
		ZeroMemory(&nTray, sizeof(nTray));
		nTray.cbSize = sizeof(nTray);
		nTray.hWnd = CreateWindowW(wc.lpszClassName, wc.lpszClassName, 0, 0, 0, 0, 0, 0, 0, histance, 0);
		nTray.uVersion = NOTIFYICON_VERSION;
		nTray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nTray.uCallbackMessage = WM_USER;
		//IDI_ICON1 - ID Иконки из файла ресурсов.
		nTray.hIcon = (HICON)LoadImageW(histance, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
		lstrcpyW(nTray.szTip, L"Fix my text");
		nTray.hBalloonIcon = (HICON)LoadImageW(histance, MAKEINTRESOURCEW(IDI_ICON2), IMAGE_ICON, 0, 0, 0);
		nTray.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON | NIIF_RESPECT_QUIET_TIME;
		lstrcpyW(nTray.szInfoTitle, L"Появилась новая версия приложения!");
		lstrcpyW(nTray.szInfo, L"Нажмите на иконку, чтобы перейти на страницу с новой версией приложения");
		Shell_NotifyIconW(NIM_ADD, &nTray);
		nTray.uFlags |= NIF_INFO;
		DestroyIcon(nTray.hIcon);
		nTray.hIcon = (HICON)LoadImageW(histance, MAKEINTRESOURCEW(IDI_ICON2), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	}
	////	Провкерка новой версии
	if (!(tempSett[BSID_NEW_VERSION] = IsNewVersionExist()))
		SetTimer(nTray.hWnd, TID_NEWVER, 900'000, NULL);	//Каждые 15 минут
	////	ВСЕ ПЕРЕМЕННЫЕ 
	StrData conservation, selected;
	ZeroMemory(&conservation, sizeof(StrData));
	ZeroMemory(&selected, sizeof(StrData));
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
		TimerWaitCycle(&msg, TID_PROBLEM, BSID_PROBLEM_KEYS);
		////	Эмуляция Ctrl + C
		AddClipboardFormatListener(nTray.hWnd);
		EmulateCombination(2, VK_CONTROL, 'C');
		TimerWaitCycle(&msg, TID_CTRLC, BSID_WAITING_CLIPBOARD);
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
		////	Самая главная функция																	<<<===
		LogicSwitch(&selected, swither);
		if (swither == LID_SWAP || swither == LID_UPDOWN)
		{
			PrintStrData(&selected, VAR_NAME(selected));
			SetBufer(&selected);
			////	Эмуляция Ctrl + V
			EmulateCombination(2, VK_CONTROL, 'V');
			////	Смена (при включеной галочки)
			switch (swither)
			{
			case LID_SWAP:
				if (fileBoolSett[BSID_CHANGE_LANG])
					PostMessageW(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_FORWARD, 0);
				break;
			case LID_UPDOWN:
				if (fileBoolSett[BSID_CHANGE_CAPS])
					EmulateCombination(1, VK_CAPITAL);
				break;
			default:
				break;
			}
			////	Ожидания действий для корректной вставки CONSERVATION
			TimerWaitCycle(&msg, TID_PROBLEM, BSID_PROBLEM_KEYS);
		}
		////	Вставка CONSERVATION
		SetBufer(&conservation);
	}
	////	Очитска
	KillTimer(nTray.hWnd, TID_NEWVER);
	DestroyWindow(nTray.hWnd);
	Shell_NotifyIconW(NIM_DELETE, &nTray);
	UnregisterClassW(WCN_ICON, histance);
	UnregisterClassW(WCN_SETT, histance);
	if (consoleWin)
		FreeConsole();
	GdiplusShutdown(token);
	ReleaseMutex(hMutex);
	return 0;
}