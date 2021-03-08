#pragma once
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <TlHelp32.h>
#include <iostream>

using namespace std;

#pragma warning(disable : 4996)
#pragma warning(disable : 6031) // 忽略警告"返回值被忽略: “getchar”"


// 不能用库函数
#define TOUPPER(x) ((((x) >= 'a') && ((x) <= 'z')) ? ((x)-32) : (x))
#define TOLOWER(x) ((((x) >= 'A') && ((x) <= 'Z')) ? ((x) + 32) : (x))
#define EXIT_ERROR(x)                                 \
    do                                                \
    {                                                 \
	cout << "error in line " << __LINE__ << endl; \
	cout << x;                                    \
	getchar();                                    \
	exit(EXIT_FAILURE);                           \
    } while (0)
#define MY_ASSERT(x)                         \
    do                                       \
{                                        \
	if (!x)                              \
	EXIT_ERROR("ASSERTION failed!"); \
    } while (0)

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;


typedef struct _PEB_LDR_DATA
{
	DWORD Length;
	bool Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	UINT32 SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	UINT32 Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	PVOID SectionPointer;
	UINT32 CheckSum;
	UINT32 TimeDateStamp;
	PVOID LoadedImports;
	PVOID EntryPointActivationContext;
	PVOID PatchInformation;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;