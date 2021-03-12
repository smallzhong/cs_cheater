// WindowsProject1.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "WindowsProject1.h"
#include "head.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



DWORD g_pid = NULL;
PCHAR g_server_css_base = NULL;
PCHAR g_server_base = NULL;
PCHAR g_client_base = NULL;
PCHAR g_engine_base = NULL;
HANDLE g_hprocess = NULL;
DWORD g_onehundred = 114514;
BOOL g_firstflag = TRUE;
PVOID g_self_blood_addr = NULL;
DWORD dwwritten = 0;
DWORD dwread = 0;
stPlayerInfo g_self_info;
stPlayerInfo g_enemy_info[10];

RECT g_stRect;
RECT g_stKhRect;
HWND g_hWnd;

DWORD WINAPI Thread_test(LPVOID lpParameter);

// 不断更新自己的信息
DWORD WINAPI Thread_get_self_info(LPVOID lpParameter)
{
	DWORD buffer = -1;
	//PVOID ptraddr = (PVOID)((PCHAR)g_server_css_base + 0x3B5D18);
	PVOID ptraddr = (PVOID)((PCHAR)g_server_css_base + 0x3D24D4);
	ReadProcessMemory(g_hprocess, ptraddr, &buffer, 4, &dwread);

	// 设置全局变量中血量地址
	g_self_blood_addr = (PVOID)((DWORD)buffer + 0x9c);

	DWORD t = -1;
	ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x8b * 4), &t, 4, &dwread);
	printf("自己阵营%d\r\n", t);

	while (1)
	{
		// 获取血量
		DWORD blood = 0;
		ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x9c), &blood, 4, &dwread);
		g_self_info.dwHp = blood;
		//printf("当前血量%d\r\n", blood);

		// 获取坐标xyz
		FLOAT x = 0, y = 0, z = 0;
		ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4 - 8), &x, 4, &dwread);
		ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4 - 4), &y, 4, &dwread);
		ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4), &z, 4, &dwread);
		//printf("x=%f y=%f z=%f\r\n", x, y, z);

		g_self_info.x = x;
		g_self_info.y = y;
		g_self_info.z = z;

		// 获取水平和垂直转轴
		FLOAT self_x = 0, self_y = 0;
		ReadProcessMemory(g_hprocess, (PVOID)(g_engine_base + 0x326EC0), &self_x, 4, &dwread);
		ReadProcessMemory(g_hprocess, (PVOID)(g_engine_base + 0x326ebc), &self_y, 4, &dwread);

		//if (self_x < 0)
		//{
		//	g_self_info.sx = self_x + 360; // 如果是负数，加上360，保证是正数，角度从0~360
		//}
		//else
		//{
		//	g_self_info.sx = self_x;
		//}
		g_self_info.sx = self_x;
		g_self_info.zy = self_y;
		//g_self_info.zy = -self_y; // 向上为正，向下为负

	//printf("hp=%d x=%f y=%f z=%f self_x=%f self_y=%f\r\n", g_self_info.dwHp, x, y, z, self_x, self_y);
	// 飞天
	//FLOAT t = 1000;
	//WriteProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4), &t, 4, &dwwritten);

		Sleep(500);
	}
}

// 不断更新敌人的信息
DWORD WINAPI Thread_enemy(LPVOID lpParameter)
{
	PCHAR buffer = NULL;
	// TODO:这里可以遍历敌人
	while (1)
	{
		for (int i = 0; i < 5; i++)
		{
			PVOID ptraddr = (PVOID)((PCHAR)g_server_css_base + 0x3D24E4 + i * 0x10);
			ReadProcessMemory(g_hprocess, ptraddr, &buffer, 4, &dwread);

			// 敌人阵营
			DWORD t = -1;
			ReadProcessMemory(g_hprocess, buffer + 0x8b * 4, &t, 4, &dwread);
			g_enemy_info[i].dwCT = t;


			// 设置敌人位置
			FLOAT x = 0, y = 0, z = 0;
			DWORD blood = 0;
			ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4 - 8), &x, 4, &dwread);
			ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4 - 4), &y, 4, &dwread);
			ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4), &z, 4, &dwread);
			g_enemy_info[i].x = x;
			g_enemy_info[i].y = y;
			g_enemy_info[i].z = z;

			// 设置敌人血量
			ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x9c), &blood, 4, &dwread);
			g_enemy_info[i].dwHp = blood;
			//printf("敌人1地址：x=%f y=%f z=%f\r\n", x, y, z);
			//printf("敌人血量：%d\r\n", blood);

			Sleep(500);
		}
	}

	return 0;
}



VOID ShowPaint(stPlayerInfo stMe, stPlayerInfo stTarget, HDC hDc, DWORD dwWindwMetricsX, DWORD dwWindwMetricsY)
{
	if (stTarget.x == 0 && stTarget.y == 0) return;
	//printf("me:x=%f y=%f z=%f hp=%d\r\n",stMe.x, stMe.y, stMe.z, stMe.dwHp);
	//printf("enemy:x=%f y=%f z=%f mex=%d mey=%d\r\n",stTarget.x, stTarget.y, stTarget.z, dwWindwMetricsX, dwWindwMetricsY);
	//printf("sx = %f, yz = %f\n", stMe.sx, stMe.zy);
	FLOAT dx = 0;
	FLOAT dy = 0;
	FLOAT dz = 0;
	//水平夹角
	FLOAT levelAngle = 0;
	//俯仰角
	FLOAT elevation = 0;
	//角色夹角
	FLOAT fPlayerAngle_xy = 0;
	FLOAT fPlayerAngle_z = 0;
	//屏幕夹角
	FLOAT fPaintAngle_xy = 0;
	FLOAT fPaintAngle_z = 0;

	//计算我和目标的相对偏移
	dx = stTarget.x - stMe.x;
	dy = stTarget.y - stMe.y;
	dz = stTarget.z - 50 - stMe.z;

	FLOAT fDistance = sqrt(dx * dx + dy * dy);

	levelAngle = stMe.sx;
	elevation = stMe.zy;
	//角度补偿  -180 180 =》0 360
	if (levelAngle < 0)
	{
		levelAngle = levelAngle + 360;
	}
	elevation = -elevation;

	//计算角色夹角
	if (dx > 0)
	{
		if (dy == 0)
		{
			fPlayerAngle_xy = 0.0;
		}
		else if (dy > 0)
		{
			fPlayerAngle_xy = asin(dy / fDistance) * 180 / 3.1416;
		}
		else if (dy < 0)
		{
			fPlayerAngle_xy = 360.0 + (asin(dy / fDistance) * 180 / 3.1416);
		}
	}
	//判断特殊角度
	else if (dx == 0)
	{
		if (dy > 0)
		{
			fPlayerAngle_xy = 90.0;
		}
		else if (dy < 0)
		{
			fPlayerAngle_xy = 270.0;
		}
	}
	else if (dx < 0)
	{
		if (dy == 0)
		{
			fPlayerAngle_xy = 180.0;
		}
		else if (dy > 0)
		{
			fPlayerAngle_xy = 180.0 - asin(dy / (fDistance)) * 180 / 3.1416;
		}
		else if (dy < 0)
		{
			fPlayerAngle_xy = 180.0 - asin(dy / (fDistance)) * 180 / 3.1416;
		}
	}

	//视角水平夹角
	fPaintAngle_xy = levelAngle - fPlayerAngle_xy;
	if (fPaintAngle_xy == 180.0 || fPaintAngle_xy == -180.0)
	{
		fPaintAngle_xy = 180.0;
	}
	else if (fPaintAngle_xy > 180.0)
	{
		fPaintAngle_xy = fPaintAngle_xy - 360.0;
	}
	else if (fPaintAngle_xy < -180.0)
	{
		fPaintAngle_xy = fPaintAngle_xy + 360.0;
	}

	//视角俯仰夹角
	fPlayerAngle_z = asin(dz / (sqrt(dx * dx + dy * dy + dz * dz))) * 180 / 3.1416;
	fPaintAngle_z = elevation - fPlayerAngle_z;


	if (fPaintAngle_xy > -45 && fPaintAngle_xy < 45 && fPaintAngle_z > -45 && fPaintAngle_z < 45)
	{
		DWORD dwLift_x;
		DWORD dwLeft_y;
		DWORD dwRight_x;
		DWORD dwRight_y;
		//屏幕坐标转换
		//FLOAT fPaintPoint = 640 * (sin(fPaintAngle_xy)*fDistance / cos(fPaintAngle_xy)*fDistance);
		FLOAT fPaintPoint = dwWindwMetricsX * (tan(fPaintAngle_xy * 3.14 / 180) + 1) / 2;
		DWORD dwWight = 100 * 100 / (100 + fDistance) + 10;

		dwLift_x = fPaintPoint - dwWight / 2;
		dwRight_x = fPaintPoint + dwWight / 2;

		//俯仰
		FLOAT fPaintSx = dwWindwMetricsY * (tan(53.0 * 3.14 / 180) * tan(fPaintAngle_z * 3.14 / 180) * 10 / 11 + 1) / 2;

		//DWORD dwHight = 100 * 512 / (100 + fDistance);
		DWORD dwHight = 100 * 300 / (fDistance);

		dwLeft_y = fPaintSx + dwHight * ((fPaintAngle_z) / 100);
		dwRight_y = fPaintSx - dwHight * (1 - (fPaintAngle_z) / 100);

		WCHAR wBuffer[20] = { 0 };
		memset(wBuffer, 0, 40);
		//swprintf_s(wBuffer, L"HP:%03d", stTarget.dwHp);
		//显示血量
		HPEN hPen = NULL;
		HBRUSH hBrush;
		//显示阵营
		if (stTarget.dwCT == 3)
		{
			hPen = CreatePen(PS_SOLID, 1, RGB(0, 245, 255));
		}
		else if (stTarget.dwCT == 2)
		{
			hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
		}
		//显示方框
		SelectObject(hDc, hPen);
		printf("left_x = %d, left_y = %d right_x = %d, right_y = %d\r\n", dwLift_x, dwLeft_y, dwRight_x, dwRight_y);
		Rectangle(hDc, dwLift_x, dwLeft_y, dwRight_x, dwRight_y);
		DeleteObject(hPen);

		// //显示血条-血条颜色设置
		// if (stTarget.dwState == 50)
		// {
		// 	SetTextColor(hDc, RGB(105, 105, 105));
		// 	hPen = CreatePen(PS_SOLID, 1, RGB(105, 105, 105));
		// 	hBrush = (HBRUSH)CreateSolidBrush(RGB(105, 105, 105));
		// }
		// else if (stTarget.dwHp >= 80)
		// {
		// 	SetTextColor(hDc, RGB(0, 255, 0));
		// 	hPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
		// 	hBrush = (HBRUSH)CreateSolidBrush(RGB(0, 255, 0));
		// }
		// else if (stTarget.dwHp >= 60)
		// {
		// 	SetTextColor(hDc, RGB(255, 255, 0));
		// 	hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
		// 	hBrush = (HBRUSH)CreateSolidBrush(RGB(255, 255, 0));
		// }
		// else if (stTarget.dwHp >= 40)
		// {
		// 	SetTextColor(hDc, RGB(255, 97, 0));
		// 	hPen = CreatePen(PS_SOLID, 1, RGB(255, 97, 0));
		// 	hBrush = (HBRUSH)CreateSolidBrush(RGB(255, 97, 0));
		// }
		// else
		// {
		// 	SetTextColor(hDc, RGB(255, 0, 0));
		// 	hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		// 	hBrush = (HBRUSH)CreateSolidBrush(RGB(255, 0, 0));
		// }
		// TextOutW(hDc, dwRight_x, dwRight_y, wBuffer, 6);

		// SelectObject(hDc, hPen);
		// Rectangle(hDc, dwLift_x, dwRight_y + 6, dwRight_x, dwRight_y);
		// DeleteObject(hPen);

		// HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);

		// Rectangle(hDc, dwLift_x, dwRight_y + 6, dwLift_x + (dwRight_x - dwLift_x) * stTarget.dwHp / 100, dwRight_y);
		// SelectObject(hDc, hOldBrush);
		// DeleteObject(hOldBrush);
		// DeleteObject(hBrush);
		// //Rectangle(hDc, 0, 0, dwWindwMetricsX, dwWindwMetricsY);
	}
}


LRESULT CALLBACK MyWindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	HDC hDc = NULL;
	MARGINS margin;
	RECT a = { 0, 50, 50, 0 };
	HBRUSH hbrush = CreateSolidBrush(RGB(0, 0, 0));
	switch (uMsg)
	{
	case WM_PAINT:

		//绘制目标人物
		hDc = GetDC(g_hWnd);
		// TODO:
		//Rectangle(hDc, 0, 0, 820, 50);
		for (int i = 0; i < 5; i++)
			ShowPaint(g_self_info, g_enemy_info[i], hDc, g_stKhRect.right - g_stKhRect.left, g_stKhRect.bottom - g_stKhRect.top);
		//ShowPlayer(hDc, g_hprocess, g_stWindowsInfo.dwBaseAddress, g_pPlayerBuffer, g_stKhRect.right - g_stKhRect.left, g_stKhRect.bottom - g_stKhRect.top);
		ReleaseDC(g_hWnd, hDc);
		break;
	case WM_CREATE:
		//区域拓展  
		margin = { g_stKhRect.left, g_stKhRect.top, g_stKhRect.right, g_stKhRect.bottom };
		//DwmExtendFrameIntoClientArea(hwnd, &margin);
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;


	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MainLoopThread()
{
	while (TRUE)
	{
		//获取DC上下文
		HDC hDc = GetDC(g_hWnd);

		//创建画刷并关联至上下文
		HBRUSH HbRUSH = CreateSolidBrush(RGB(255, 255, 255));
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, HbRUSH);
		//根据窗口重新绘制矩形
		Rectangle(hDc, 0, 0, g_stKhRect.right - g_stKhRect.left, g_stKhRect.bottom - g_stKhRect.top);

		HANDLE hMutex = CreateMutex(NULL, FALSE, TEXT("数据轮询"));
		SendMessage(g_hWnd, WM_PAINT, NULL, NULL);
		ReleaseMutex(hMutex);
		Sleep(50);
		SelectObject(hDc, hOldBrush);
		//清理句柄
		DeleteObject(HbRUSH);
		DeleteObject(hOldBrush);
		ReleaseDC(g_hWnd, hDc);
	}
}

DWORD WINAPI Thread_test(LPVOID lpParameter)
{


	//创建窗口
	WNDCLASSEX WINCLASS = { sizeof WNDCLASSEX };

	WINCLASS.cbSize = sizeof(WNDCLASSEX);
	WINCLASS.style = CS_HREDRAW | CS_VREDRAW;//改变则重绘 
	WINCLASS.lpfnWndProc = MyWindowProc;
	WINCLASS.hInstance = GetModuleHandle(0);
	WINCLASS.hCursor = LoadCursor(NULL, IDC_ARROW);
	WINCLASS.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	WINCLASS.lpszClassName = L"CS-Cover";
	RegisterClassEx(&WINCLASS);

	//获取目标窗口
	HWND hCsWnd = FindWindow(NULL, L"Counter-Strike Source");

	//获得窗口大小位置及(分辨率)
	GetWindowRect(hCsWnd, &g_stRect);
	GetClientRect(hCsWnd, &g_stKhRect);
	g_stKhRect.left = g_stRect.left;
	g_stKhRect.right = g_stRect.right;
	g_stKhRect.top = g_stRect.bottom - g_stKhRect.bottom;
	g_stKhRect.bottom = g_stRect.bottom;

	//获取窗口进程信息
	//InitWindowsCS();

	g_hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, WINCLASS.lpszClassName, L"CS-Cover",
		WS_POPUP, g_stKhRect.left, g_stKhRect.top, g_stKhRect.right - g_stKhRect.left,
		g_stKhRect.bottom - g_stKhRect.top, NULL, NULL, WINCLASS.hInstance, NULL);

	//分层窗口透明度和颜色筛选
	SetLayeredWindowAttributes(g_hWnd, RGB(255, 255, 255), 0, LWA_COLORKEY);
	//SetLayeredWindowAttributes(g_hWnd, 0, 255, LWA_ALPHA);

	// 展示窗口
	ShowWindow(g_hWnd, SW_SHOW);

	//完全置顶
	SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainLoopThread, (PDWORD)&g_hWnd, 0, NULL);


	//消息循环
	MSG msg = { 0 };
	while (GetMessage(&msg, 0, 0, 0))
	{
		DispatchMessage(&msg);
	}
	CloseHandle(g_hprocess);

	return 0;
}





BOOL CALLBACK MainDlgProc(HWND hwndDlg,   // handle to dialog box
	UINT uMsg,      // message
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON1:
		{
			::CreateThread(NULL, 0, Thread_test, NULL, 0, NULL);
		}
		}
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		exit(0);
		break;

	default:
		break;
	}
	return FALSE;
}

VOID getProcessPID()
{
	HANDLE hProceessnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProceessnap == INVALID_HANDLE_VALUE)
		EXIT_ERROR("创建进程快照失败\n");
	else
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(pe32);
		BOOL hProcess = Process32First(hProceessnap, &pe32);
		while (hProcess)
		{
			//printf("%ws\r\n", pe32.szExeFile);
			if (!wcscmp(pe32.szExeFile, TEXT("hl2.exe")))
			{
				//printf("id = 0x%x\r\n", pe32.th32ProcessID);
				g_pid = (DWORD)(pe32.th32ProcessID);
				printf("成功找到进程！! %ws pid = %x\n", pe32.szExeFile, g_pid);
				break;
			}
			//printf("%ws 0x%x\n", pe32.szExeFile, pe32.th32ProcessID);
			hProcess = Process32Next(hProceessnap, &pe32);
		}
	}
	if (g_pid == NULL)
		EXIT_ERROR("没有成功找到CS进程！\r\n");
	CloseHandle(hProceessnap);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	//UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。
	//DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, MainDlgProc);

	// 开辟控制台输出调试信息 
	AllocConsole();
	SetConsoleTitle(_T("Debug Output"));
	freopen("CONOUT$", "w", stdout);

	// 
	getProcessPID();
	g_hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, g_pid);
	if (g_hprocess == NULL)
		EXIT_ERROR("打开进程失败！\r\n");

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, g_pid);
	if (INVALID_HANDLE_VALUE == hSnapshot)
		EXIT_ERROR("创建模块快照失败\n");
	MODULEENTRY32 mi;
	mi.dwSize = sizeof(MODULEENTRY32); //第一次使用必须初始化成员
	BOOL bRet = Module32First(hSnapshot, &mi);
	while (bRet)
	{
		/*printf("模块名：%ws，size = 0x%x，baseaddr = 0x%x\r\n",
			mi.szModule, mi.dwSize, (DWORD)mi.modBaseAddr);*/

		if (!wcscmp(mi.szModule, TEXT("server_css.dll")))
		{
			printf("成功找到模块server_css.dll，基址为0x%p，大小为0x%x\r\n", mi.modBaseAddr, (DWORD)mi.modBaseSize);
			g_server_css_base = (PCHAR)mi.modBaseAddr;
		}

		if (!wcscmp(mi.szModule, TEXT("client.dll")))
		{
			printf("成功找到模块client.dll，基址为0x%p，大小为0x%x\r\n", mi.modBaseAddr, (DWORD)mi.modBaseSize);
			g_client_base = (PCHAR)mi.modBaseAddr;
		}

		if (!wcscmp(mi.szModule, TEXT("server.dll")))
		{
			printf("成功找到模块server.dll，基址为0x%p，大小为0x%x\r\n", mi.modBaseAddr, mi.modBaseSize);
			g_server_base = (PCHAR)mi.modBaseAddr;
		}

		if (!wcscmp(mi.szModule, TEXT("engine.dll")))
		{
			printf("成功找到模块engine.dll，基址为0x%p，大小为0x%x\r\n", mi.modBaseAddr, mi.modBaseSize);
			g_engine_base = (PCHAR)mi.modBaseAddr;
		}

		bRet = Module32Next(hSnapshot, &mi);
	}

	if (g_server_css_base == NULL || g_client_base == NULL)
		EXIT_ERROR("没有找到server_css.dll模块或没有找到client模块！\r\n");

	::CreateThread(NULL, 0, Thread_get_self_info, NULL, 0, NULL);
	::CreateThread(NULL, 0, Thread_enemy, NULL, 0, NULL);

	HANDLE th_test = ::CreateThread(NULL, 0, Thread_test, NULL, 0, NULL);


	::WaitForSingleObject(th_test, INFINITE);
	return 0;

	// 初始化全局字符串
	//LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
	//MyRegisterClass(hInstance);

	//// 执行应用程序初始化:
	//if (!InitInstance(hInstance, nCmdShow))
	//{
	//	return FALSE;
	//}

	//HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

	//MSG msg;

	//// 主消息循环:
	//while (GetMessage(&msg, nullptr, 0, 0))
	//{
	//	if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
	//	{
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//	}
	//}

	//return (int)msg.wParam;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

	ShowWindow(hWnd, nCmdShow);
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
