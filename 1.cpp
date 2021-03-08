#include "head.h"

using namespace std;

DWORD g_pid = NULL;

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

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, g_pid);
	if (INVALID_HANDLE_VALUE == hSnapshot)
		EXIT_ERROR("创建模块快照失败\n");
	MODULEENTRY32 mi;
	mi.dwSize = sizeof(MODULEENTRY32); //第一次使用必须初始化成员
	BOOL bRet = Module32First(hSnapshot, &mi);
	while (bRet)
	{
		printf("模块名：%ws，size = 0x%x，baseaddr = 0x%x\r\n",
			mi.szModule, mi.dwSize, (DWORD)mi.modBaseAddr);

		bRet = Module32Next(hSnapshot, &mi);
	}
	return 0;

}