#include "head.h"

DWORD g_pid = NULL;
PCHAR g_server_css_base = NULL;
PCHAR g_server_base = NULL;
PCHAR g_client_base = NULL;
HANDLE g_hprocess = NULL;
DWORD g_onehundred = 200;
BOOL g_firstflag = TRUE;
DWORD dwwritten = 0;
DWORD dwread = 0;

DWORD WINAPI Thread_blood_lock(LPVOID lpParameter);
DWORD WINAPI Thread_test(LPVOID lpParameter);
VOID getProcessPID();

int main()
{
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

		bRet = Module32Next(hSnapshot, &mi);
	}

	if (g_server_css_base == NULL || g_client_base == NULL)
		EXIT_ERROR("没有找到server_css.dll模块或没有找打client模块！\r\n");

	// 开启锁血线程
	HANDLE th_blood_lock = ::CreateThread(NULL, 0, Thread_blood_lock, NULL, 0, NULL);
	if (th_blood_lock == NULL)
		EXIT_ERROR("启动锁血线程失败！\r\n");
	else
		printf("开启锁血线程成功！\r\n");

	// 测试线程
	//HANDLE th_test = ::CreateThread(NULL, 0, Thread_test, NULL, 0, NULL);

	// 等待锁血线程结束（当然这里永远不会结束）
	WaitForSingleObject(th_blood_lock, INFINITE);

	return 0;
}

DWORD WINAPI Thread_test(LPVOID lpParameter)
{
	PCHAR pMyBase = NULL;
	FLOAT myx = 0, myy = 0, myz = 0;

	ReadProcessMemory(g_hprocess, g_server_base + 0x4f2fec, &pMyBase, 4, &dwread);
	printf("pmybase = 0x%p\r\n", pMyBase);

	while (1)
	{
		ReadProcessMemory(g_hprocess, pMyBase + 0x288 - 8, &myx, 4, &dwread);
		ReadProcessMemory(g_hprocess, pMyBase + 0x288 - 4, &myy, 4, &dwread);
		ReadProcessMemory(g_hprocess, pMyBase + 0x288, &myz, 4, &dwread);

		printf("%f %f %f\r\n", myx, myy, myz);
		Sleep(500);
	}
	/*ReadProcessMemory(g_hprocess, g_client_base + 0x2CC078, &pMyBase, 4, &dwread);

	printf("pMyBase = 0x%p\r\n", pMyBase);

	while (1)
	{
		ReadProcessMemory(g_hprocess, pMyBase + 0x2c8, &myx, 4, &dwread);
		ReadProcessMemory(g_hprocess, pMyBase + 0x2cc, &myx, 4, &dwread);
		ReadProcessMemory(g_hprocess, pMyBase + 0x2d0, &myx, 4, &dwread);

		printf("%f %f %f\r\n", myx, myy, myz);
		Sleep(500);
	}	*/
	return 0;
}

DWORD WINAPI Thread_blood_lock(LPVOID lpParameter)
{
	while (1)
	{
		DWORD buffer = -1;

		//  + 0x9c
		PVOID ptraddr = (PVOID)((PCHAR)g_server_css_base + 0x3B5D18);
		if (g_firstflag)
			printf("存血量结构体指针基址 = 0x%x\r\n", ptraddr);

		ReadProcessMemory(g_hprocess, ptraddr, &buffer, 4, &dwread);
		if (g_firstflag)
		{
			printf("存血量结构体地址：0x%x\r\n", buffer);
			g_firstflag = FALSE;
		}
		DWORD blood = -1;
		ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x9c), &blood, 4, &dwread);
		FLOAT x = 0, y = 0, z = 0;
		ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4 - 8), &x, 4, &dwread);
		ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4 - 4), &y, 4, &dwread);
		ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4), &z, 4, &dwread);

		FLOAT t = 1000;
		WriteProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x2c4), &t, 4, &dwwritten);


		printf("x=%f y=%f z=%f\r\n", x, y, z);
		if (blood != 100)
		{
			//printf("当前血量%d\r\n", blood);
			WriteProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x9c), &g_onehundred, 4, &dwwritten);
			//printf("\t血量恢复！\r\n");
		}
		else
		{
			WriteProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x9c), &g_onehundred, 4, &dwwritten);
		}
		//WriteProcessMemory(g_hprocess, (PVOID)((PCHAR)g_server_css_base + 0x3B5D18 + 0x9c), &onehundred, 4, &dwwritten);
		//printf("%d字节写入成功！\r\n", dwwritten);

		Sleep(1000);
	}

	return 0;
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
