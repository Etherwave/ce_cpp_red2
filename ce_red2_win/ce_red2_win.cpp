// ce_red2_win.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "ce_red2_win.h"

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include <windows.h>
#include <Winuser.h>
#include <string>
#include <fstream>
#include<mmsystem.h>
#pragma comment(lib, "WINMM.LIB")

#include "functions.h"
#include "red2_functions.h"

using namespace std;
const int KeyBoardValue = 0x80000000;

string GetKeyName(int);
bool JudgeShift();
DWORD WINAPI red2(LPVOID lparam);

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

string process_name = "game.exe";

InjectCodeManager inject_code_manager_money, inject_code_manager_zero_cooldown_time;


int pid;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CERED2WIN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CERED2WIN));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CERED2WIN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CERED2WIN);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   //ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        ::CreateThread(
            NULL,
            NULL,
            red2,
            NULL,
            0,
            NULL
        );
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

string GetKeyName(int NumKey)
{
	bool IS_SHIFT = JudgeShift();
	string revalue = "";
	//判断键盘中间的特殊符号
	if (NumKey >= 186 && NumKey <= 222)
		switch (NumKey)
		{
		case 186:
			if (IS_SHIFT)
				revalue = ":";
			else
				revalue = ";";
			break;
		case 187:
			if (IS_SHIFT)
				revalue = "+";
			else
				revalue = "=";
			break;
		case 188:
			if (IS_SHIFT)
				revalue = "<";
			else
				revalue = ",";
			break;
		case 189:
			if (IS_SHIFT)
				revalue = "_";
			else
				revalue = "-";
			break;
		case 190:
			if (IS_SHIFT)
				revalue = ">";
			else
				revalue = ".";
			break;
		case 191:
			if (IS_SHIFT)
				revalue = "?";
			else
				revalue = "/";
			break;
		case 192:
			if (IS_SHIFT)
				revalue = "~";
			else
				revalue = "`";
			break;
		case 219:
			if (IS_SHIFT)
				revalue = "{";
			else
				revalue = "[";
			break;
		case 220:
			if (IS_SHIFT)
				revalue = "|";
			else
				revalue = "\\";
			break;
		case 221:
			if (IS_SHIFT)
				revalue = "}";
			else
				revalue = "]";
			break;
		case 222:
			if (IS_SHIFT)
				revalue = '"';
			else
				revalue = ",";
		default:
			revalue = "error";
			break;
		}


	if (NumKey == VK_ESCAPE) // 退出
		revalue = "[Esc]";
	else if (NumKey == VK_F1) // F1至F12
		revalue = "[F1]";
	else if (NumKey == VK_F2)
		revalue = "[F2]";
	else if (NumKey == VK_F3)
		revalue = "[F3]";
	else if (NumKey == VK_F4)
		revalue = "[F4]";
	else if (NumKey == VK_F5)
		revalue = "[F5]";
	else if (NumKey == VK_F6)
		revalue = "[F6]";
	else if (NumKey == VK_F7)
		revalue = "[F7]";
	else if (NumKey == VK_F8)
		revalue = "[F8]";
	else if (NumKey == VK_F9)
		revalue = "[F9]";
	else if (NumKey == VK_F10)
		revalue = "[F10]";
	else if (NumKey == VK_F11)
		revalue = "[F11]";
	else if (NumKey == VK_F12)
		revalue = "[F12]";
	else if (NumKey == VK_SNAPSHOT) // 打印屏幕
		revalue = "[PrScrn]";
	else if (NumKey == VK_SCROLL) // 滚动锁定
		revalue = "[Scroll Lock]";
	else if (NumKey == VK_PAUSE) // 暂停、中断
		revalue = "[Pause]";
	else if (NumKey == VK_CAPITAL) // 大写锁定
		revalue = "[Caps Lock]";
	else if (NumKey == 8) //<- 回格键
		revalue = "[Backspace]";
	else if (NumKey == VK_RETURN) // 回车键、换行
		revalue = "[Enter]\n";
	else if (NumKey == VK_SPACE) // 空格
		revalue = " ";
	else if (NumKey == VK_TAB) // 制表键
		revalue = "[Tab]";
	else if (NumKey == VK_LCONTROL) // 左控制键
		revalue = "[Ctrl]";
	else if (NumKey == VK_RCONTROL) // 右控制键
		revalue = "[CTRL]";
	else if (NumKey == VK_LMENU) // 左换档键
		revalue = "[Alt]";
	else if (NumKey == VK_LMENU) // 右换档键
		revalue = "[ALT]";
	else if (NumKey == VK_LWIN) // 右 WINDOWS 键
		revalue = "[Win]";
	else if (NumKey == VK_RWIN) // 右 WINDOWS 键
		revalue = "[WIN]";
	else if (NumKey == VK_APPS) // 键盘上 右键
		revalue = "右键";
	else if (NumKey == VK_INSERT) // 插入
		revalue = "[Insert]";
	else if (NumKey == VK_DELETE) // 删除
		revalue = "[Delete]";
	else if (NumKey == VK_HOME) // 起始
		revalue = "[Home]";
	else if (NumKey == VK_END) // 结束
		revalue = "[End]";
	else if (NumKey == VK_PRIOR) // 上一页
		revalue = "[PgUp]";
	else if (NumKey == VK_NEXT) // 下一页
		revalue = "[PgDown]";
	// 不常用的几个键:一般键盘没有
	else if (NumKey == VK_CANCEL) // Cancel
		revalue = "[Cancel]";
	else if (NumKey == VK_CLEAR) // Clear
		revalue = "[Clear]";
	else if (NumKey == VK_SELECT) //Select
		revalue = "[Select]";
	else if (NumKey == VK_PRINT) //Print
		revalue = "[Print]";
	else if (NumKey == VK_EXECUTE) //Execute
		revalue = "[Execute]";

	//----------------------------------------//
	else if (NumKey == VK_LEFT) //上、下、左、右键
		revalue = "[←]";
	else if (NumKey == VK_RIGHT)
		revalue = "[→]";
	else if (NumKey == VK_UP)
		revalue = "[↑]";
	else if (NumKey == VK_DOWN)
		revalue = "[↓]";
	else if (NumKey == VK_NUMLOCK)//小键盘数码锁定
		revalue = "[NumLock]";
	else if (NumKey == VK_ADD) // 加、减、乘、除
		revalue = "+";
	else if (NumKey == VK_SUBTRACT)
		revalue = "-";
	else if (NumKey == VK_MULTIPLY)
		revalue = "*";
	else if (NumKey == VK_DIVIDE)
		revalue = "/";
	else if (NumKey == 190 || NumKey == 110) // 小键盘 . 及键盘 .
		revalue = ".";
	//小键盘数字键:0-9
	else if (NumKey == VK_NUMPAD0)
		revalue = "0";
	else if (NumKey == VK_NUMPAD1)
		revalue = "1";
	else if (NumKey == VK_NUMPAD2)
		revalue = "2";
	else if (NumKey == VK_NUMPAD3)
		revalue = "3";
	else if (NumKey == VK_NUMPAD4)
		revalue = "4";
	else if (NumKey == VK_NUMPAD5)
		revalue = "5";
	else if (NumKey == VK_NUMPAD6)
		revalue = "6";
	else if (NumKey == VK_NUMPAD7)
		revalue = "7";
	else if (NumKey == VK_NUMPAD8)
		revalue = "8";
	else if (NumKey == VK_NUMPAD9)
		revalue = "9";
	//----------------------------上述代码判断键盘上除了字母之外的功能键--------------------------------//
	else if (NumKey >= 65 && NumKey <= 90)
	{
		if (GetKeyState(VK_CAPITAL))
		{
			if (IS_SHIFT)
				revalue = NumKey + 32;
			else
				revalue = NumKey;
		}
		else
		{
			if (IS_SHIFT)
				revalue = NumKey;
			else
				revalue = NumKey + 32;
		}
	}
	//---------------------------上面的部分判断键盘上的字母----------------------------------------------//
	else if (NumKey >= 48 && NumKey <= 57)
	{
		if (IS_SHIFT)
		{
			switch (NumKey)
			{
			case 48:revalue = ")"; break;
			case 49:revalue = "!"; break;
			case 50:revalue = "@"; break;
			case 51:revalue = "#"; break;
			case 52:revalue = "$"; break;
			case 53:revalue = "%"; break;
			case 54:revalue = "^"; break;
			case 55:revalue = "&"; break;
			case 56:revalue = "*"; break;
			case 57:revalue = "("; break;
			}
		}
		else
		{
			switch (NumKey)
			{
			case 48:revalue = "0"; break;
			case 49:revalue = "1"; break;
			case 50:revalue = "2"; break;
			case 51:revalue = "3"; break;
			case 52:revalue = "4"; break;
			case 53:revalue = "5"; break;
			case 54:revalue = "6"; break;
			case 55:revalue = "7"; break;
			case 56:revalue = "8"; break;
			case 57:revalue = "9"; break;
			}
		}
	}
	return revalue;
}

bool JudgeShift()
{
	int   iShift = GetKeyState(0x10); //判断Shift键状态
	bool   IS = (iShift & KeyBoardValue) == KeyBoardValue; //表示按下Shift键
	if (IS)
		return 1;
	else
		return 0;
}

void init()
{
	pid = FindPID(process_name);
	while (pid == -1)
	{
		pid = FindPID(process_name);
		string message_str = "未检测到 game.exe";
		MessageBoxA(NULL, message_str.c_str(), message_str.c_str(), MB_OK);
		Sleep(5000);//暂停5秒
	}
}

void play_switch_sound(bool flag)
{
	if(flag)PlaySound(TEXT("bombpl.wav"), 0, SND_FILENAME);
	else PlaySound(TEXT("bombdef.wav"), 0, SND_FILENAME);
}

DWORD WINAPI red2(LPVOID lparam)
{
	init();
	bool inject_money_decrease_switch = FALSE, inject_zero_cooldown_time_switch = FALSE, unlimted_blood_switch = FALSE, upgrade_switch = FALSE, rebellion_switch = FALSE;
	while (1)
	{
		for (int i = 112; i <= 123; i++)
		{
			// 112:F1 123:F12
			if (GetAsyncKeyState(i) & 1)               //判断虚拟按键是否按下，无论是一直按着还是按一下就弹起，只判断是否按过
			{
				switch (i)
				{
					// F1 加钱
				case 112:inject_money_decrease_switch = !inject_money_decrease_switch; inject_money_decrease(pid, inject_code_manager_money); play_switch_sound(inject_money_decrease_switch); break;
					// F2 无冷却
				case 113:inject_zero_cooldown_time_switch = !inject_zero_cooldown_time_switch; inject_zero_cooldown_time(pid, inject_code_manager_zero_cooldown_time); play_switch_sound(inject_zero_cooldown_time_switch);  break;
					// F3 选中的自己对象加血
				case 114: unlimted_blood_switch = !unlimted_blood_switch; play_switch_sound(unlimted_blood_switch); break;
					// F4 选中的自己对象升级
				case 115: upgrade_switch = !upgrade_switch; play_switch_sound(upgrade_switch); break;
					// F5 选中的对方对象叛变
				case 116: rebellion_switch = !rebellion_switch; play_switch_sound(rebellion_switch); break;
				default: break;
				}
			}
		}
		if (unlimted_blood_switch)unlimted_blood(pid);
		if (upgrade_switch)upgrade(pid);
		if (rebellion_switch)rebellion(pid);
	}
	return 0;
}