#include "head.h"

DWORD g_pid = NULL;
PVOID g_server_css_base = NULL;
HANDLE g_hprocess = NULL;

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
			printf("%ws\r\n", pe32.szExeFile);
			if (!wcscmp(pe32.szExeFile, TEXT("hl2.exe")))
			{
				printf("id = 0x%x\r\n", pe32.th32ProcessID);
				g_pid = (DWORD)(pe32.th32ProcessID);
				printf("成功找到进程！! %ws %x\n", pe32.szExeFile, g_pid);
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
			printf("成功找到模块server_css.dll，基址为0x%x，大小为0x%x\r\n", mi.modBaseAddr, (DWORD)mi.modBaseSize);
			g_server_css_base = mi.modBaseAddr;
			break;
		}

		bRet = Module32Next(hSnapshot, &mi);
	}

	if (g_server_css_base == NULL)
		EXIT_ERROR("没有找到server_css.dll模块！\r\n");


	while (1)
	{
		DWORD buffer = -1;
		DWORD dwwritten = 0;
		DWORD dwread = 0;

		//  + 0x9c
		PVOID ptraddr = (PVOID)((PCHAR)g_server_css_base + 0x3B5D18);
		printf("基址 = 0x%x\r\n", ptraddr);

		ReadProcessMemory(g_hprocess, ptraddr, &buffer, 4, &dwread);
		printf("存血量结构体地址：0x%x\r\n", buffer);

		DWORD blood = -1;
		ReadProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x9c), &blood, 4, &dwread);
		printf("当前血量%d\r\n", blood);

		DWORD onehundred = 100;
		WriteProcessMemory(g_hprocess, (PVOID)((PCHAR)buffer + 0x9c), &onehundred, 4, &dwwritten);
		//WriteProcessMemory(g_hprocess, (PVOID)((PCHAR)g_server_css_base + 0x3B5D18 + 0x9c), &onehundred, 4, &dwwritten);
		//printf("%d字节写入成功！\r\n", dwwritten);

		Sleep(1000);
	}

	return 0;

}