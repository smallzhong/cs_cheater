// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <timeapi.h>
#include <windows.h>
#include <stdio.h>
#pragma comment(lib,"Winmm.lib")
#pragma warning(disable:4996)

typedef BOOL(*zyc_QueryPerformanceCounter)(__out LARGE_INTEGER* lpPerformanceCount);
typedef DWORD(*zyc_2)(VOID);
zyc_2 g_oldGetTickCount;
zyc_2 g_oldTimeGetTime;
zyc_QueryPerformanceCounter g_oldQueryPerformanceCounter;

typedef int (WINAPI* Pmessagebox)(HWND, LPCTSTR, LPCTSTR, UINT);
Pmessagebox oldAddr;

DWORD m_s_iSpeedTimes = 10;

DWORD WINAPI NewTimeGetTime(void)
{
	printf("1");
	static DWORD fake = 0;
	static DWORD last_real = 0;
	DWORD now = g_oldTimeGetTime();
	DWORD result;

	if (last_real == 0)
	{
		result = fake = last_real = now;
	}
	else
	{
		result = fake + m_s_iSpeedTimes * (now - last_real);
		fake = result;
		last_real = now;
	}

	return result;
}

DWORD WINAPI NewGetTickCount(void)
{
	printf("2");
	static DWORD fake = 0;
	static DWORD last_real = 0;
	DWORD now = g_oldGetTickCount();
	DWORD result;

	if (last_real == 0)
	{
		result = fake = last_real = now;
	}
	else
	{
		result = fake + m_s_iSpeedTimes * (now - last_real);
		fake = result;
		last_real = now;
	}

	return result;
}

BOOL WINAPI NewQueryPerformanceCounter(__out LARGE_INTEGER* lpPerformanceCount)
{
	printf("3");
	BOOL ret = g_oldQueryPerformanceCounter(lpPerformanceCount);
	if (!ret) return ret;

	static LARGE_INTEGER fake = { 0 };
	static LARGE_INTEGER last_real = { 0 };
	LARGE_INTEGER now = *lpPerformanceCount;

	if (last_real.QuadPart == 0)
	{
		fake = last_real = now;
	}
	else
	{
		lpPerformanceCount->QuadPart = fake.QuadPart + m_s_iSpeedTimes * (now.QuadPart - last_real.QuadPart);
		fake = *lpPerformanceCount;
		last_real = now;
	}

	return ret;
}

DWORD WINAPI fake(HWND hWnd,          // handle to owner window
	LPCTSTR lpText,     // text in message box
	LPCTSTR lpCaption,  // message box title
	UINT uType          // message box style);
)
{
	oldAddr(0, 0, 0, 0);
	return 0;
}

/************************************
**	IAT_HOOK功能函数
**	DWORD	pFunAddre	：目标API函数地址
**	DWORD	pMyAddre	：替换的HOOK函数地址
**	PDWORD	pOldAddre	：返回的原来的IAT地址
************************************/
DWORD  SetApiHook(DWORD pFunAddre, DWORD pMyAddre, PDWORD pOldAddre)
{
	DWORD dwLastSet = 0;
	PIMAGE_DOS_HEADER pDos = NULL;
	PIMAGE_NT_HEADERS32 pNT = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pInport = NULL;
	PDWORD pIAT = NULL;

	//获得模块基址
	//HMODULE hModBaseAdd = GetModuleHandleA(NULL);
	HMODULE hModBaseAdd = GetModuleHandleA("hl2.exe");

	//找到目标导入表
	pDos = (PIMAGE_DOS_HEADER)hModBaseAdd;
	pNT = (PIMAGE_NT_HEADERS32)((PCHAR)pDos + pDos->e_lfanew);
	pInport = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hModBaseAdd + (DWORD)pNT->OptionalHeader.DataDirectory[1].VirtualAddress);

	//遍历IAT
	while (pInport->FirstThunk != 0)
	{
		pIAT = (PDWORD)((DWORD)hModBaseAdd + (DWORD)pInport->FirstThunk);

		while (*pIAT != 0)
		{
			if (*pIAT == (DWORD)pFunAddre)
			{
				//开始HOOK
				printf("piat = 0x%p\r\n", pIAT);
				VirtualProtect(pIAT, sizeof(PDWORD), PAGE_EXECUTE_READWRITE, &dwLastSet);
				*pOldAddre = (DWORD)*pIAT;
				*pIAT = (DWORD)pMyAddre;
				//结束HOOK
				//VirtualProtect(pIAT, sizeof(PDWORD), dwLastSet, NULL);
				return 0;
			}
			pIAT += 1;
		}
		pInport += 1;
	}
	return 0;
}

VOID init()
{
	DWORD old = 0;
	printf("原timegettime地址0x%p，新地址", timeGetTime);
	SetApiHook((DWORD)timeGetTime, (DWORD)NewTimeGetTime, &old);
	printf("0x%p, iat地址0x%x\r\n", timeGetTime, old);

	printf("原GetTickCount地址0x%p，新地址", GetTickCount);
	SetApiHook((DWORD)GetTickCount, (DWORD)NewGetTickCount, &old);
	printf("0x%p, iat地址0x%x\r\n", GetTickCount, old);

	printf("原NewQueryPerformanceCounter地址0x%p，新地址", QueryPerformanceCounter);
	SetApiHook((DWORD)QueryPerformanceCounter, (DWORD)NewQueryPerformanceCounter, &old);
	printf("0x%p，iat地址0x%x\r\n", QueryPerformanceCounter, old);

	printf("原MessageBoxA地址0x%p，新地址", MessageBoxA);
	SetApiHook((DWORD)MessageBoxA, (DWORD)fake, &old);
	printf("0x%p，iat地址0x%x\r\n", MessageBoxA, old);



	GetTickCount();
	timeGetTime();
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);

	while (1){
	
		MessageBoxA(0, "没被hook", "没被hook", 0);
		old = GetTickCount();
		printf("t = %d\r\n", old);
		Sleep(500);
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		AllocConsole();
		SetConsoleTitle(TEXT("Debug Output"));
		freopen("CONOUT$", "w", stdout);
        init();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

