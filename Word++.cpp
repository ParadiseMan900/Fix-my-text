#include <windows.h>
#include <iostream>

int switcher = 0;

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
	if ((letter >= 97 && letter <= 122)
		|| (letter >= -32 && letter <= -1))
	{
		letter = letter + ('A' - 'a');
	}
	if (letter == 'ё')
		letter = 'Ё';
	return letter;
}
char LDown(char letter)
{
	if ((letter >= 65 && letter <= 90)
		|| (letter >= -64 && letter <= -33))
	{
		letter = letter + ('a' - 'A');
	}
	if (letter == 'Ё')
		letter = 'ё';
	return letter;
}
char LSwap(char ch)
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
	if (ch == '?')
		return '&';
}
void Logic(char* selected)
{
	if (selected != NULL)
		switch (switcher)
		{
		case 2:
			for (int i = 0; i < strlen(selected); i++)
				selected[i] = LUp(selected[i]);
			break;
		case 3:
			for (int i = 0; i < strlen(selected); i++)
				selected[i] = LDown(selected[i]);
			break;
		case 4:
			for (int i = 0; i < strlen(selected); i++)
			{
				if (selected[i] != LDown(selected[i]))
				{
					selected[i] = LDown(selected[i]);
					selected[i] = LSwap(selected[i]);
					selected[i] = LUp(selected[i]);
				}
				else selected[i] = LSwap(selected[i]);
			}
			break;
		default:
			return;
		}
	SetBufer(selected);
}
/////////////////////////////////////////////////////////////////////////////////////
struct keysComb
{
	int* muss;
	int len;
	int ID;
};
bool keysPress(keysComb comb)
{
	for (int i = 0; i < comb.len; i++)
	{
		if (!GetAsyncKeyState(comb.muss[i]))
			return false;
	}
	switcher = comb.ID;
	return true;
}
void EmulateACombinationWithCrl(char key)
{
	INPUT input[2] = { 0 };
	int keysMuss[] = { VK_CONTROL, key };
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = VK_CONTROL;
	input[0].ki.dwFlags = 0;
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = key;
	input[1].ki.dwFlags = 0;
	SendInput(2, input, sizeof(INPUT));
	Sleep(10);
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;
	input[0].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(2, input, sizeof(INPUT));
}

int main()
{
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false);
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	//Список ключей клавиатуры:
	//https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	
	int keyExit[] = { VK_ESCAPE };
	int keyExitLen = sizeof(keyExit) / sizeof(keyExit[0]);
	keysComb toEXIT = { keyExit , keyExitLen , 1 };

	int keysUp[] = { VK_LWIN, VK_F12 };
	int keyUpLen = sizeof(keysUp) / sizeof(keysUp[0]);
	keysComb toUP = { keysUp , keyUpLen , 2 };

	int keysDown[] = { VK_LWIN, VK_F11 };
	int keyDownLen = sizeof(keysDown) / sizeof(keysDown[0]);
	keysComb toDOWN = { keysDown , keyDownLen , 3 };
	
	int keysLang[] = { VK_LWIN, VK_F4 };
	int keyLangLen = sizeof(keysLang) / sizeof(keysLang[0]);
	keysComb toSWAP = { keysLang , keyLangLen , 4 };

	int exitTime = 2000;    //2 Секунды для выхода
	int time = 0;
	char* conservation = NULL;
	char* bufer = NULL;
	char* selected = NULL;
	while (true)
	{
		if (keysPress(toUP)|| keysPress(toDOWN)|| keysPress(toSWAP))
		{
			//GET CONSERVATION;
			bufer = (char*)GetBufer(true);
			ClearBufer();
			if (bufer != NULL)
			{
				conservation = new char[strlen(bufer) + 1];
				memcpy(conservation, bufer, strlen(bufer));
				conservation[strlen(bufer)] = '\0';
			}

			//LOGIC (CTRL+C / GET / DO STAF / SET / CTRL+V);
			selected = NULL;
			while (keysPress(toUP) || keysPress(toDOWN) || keysPress(toSWAP)) {}
			for (int i = 0; i < 25; i++)
			{
				EmulateACombinationWithCrl('C');
				selected = (char*)GetBufer(false);
				if (selected != NULL)
					break;
			} 
			selected = (char*)GetBufer(true);
			ClearBufer();
			Logic(selected);                                        //<<<===
			EmulateACombinationWithCrl('V');

			//SET CONSERVATION;
			SetBufer(conservation);
			printf("---------\n");
		}
		if (keysPress(toEXIT))
		{
			time++;
			printf("%d\n", time);
			if (time * 10 >= exitTime)
				break;
		}
		else time = 0;
		Sleep(10);
	}
	system("cls");
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), true);
	printf("Программа закрывается...");
	Sleep(1000);
	free(bufer);
	free(conservation);
	free(selected);
}