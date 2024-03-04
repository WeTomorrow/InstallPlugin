#define _NO_CRT_STDIO_INLINE
#include <Windows.h>
#include <Shldisp.h> ///Shldisp.idl
#include <stdio.h>
#include <locale.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "User32.lib")
//#pragma comment(lib, "kernel32.lib")
//#pragma comment(lib, "msvcrt64.lib")
//#pragma comment(linker, "/ENTRY:main")


//提权资源文件
extern "C"  BOOL res2file(HMODULE hMod, INT RES_ID, wchar_t* RES_NAME, wchar_t* FilePath)
{
    BOOL ST = FALSE;

    if (hMod == NULL) hMod = GetModuleHandle(NULL);
    HRSRC hRsrc = FindResourceW(hMod, MAKEINTRESOURCEW(RES_ID), RES_NAME);
    if (hRsrc)
    {
        HGLOBAL hGlobal = LoadResource(hMod, hRsrc);
        if (hGlobal)
        {
            LPVOID lpAddress = LockResource(hGlobal);
            int nSize = SizeofResource(hMod, hRsrc);
            if (nSize > 0)
            {
                HANDLE hFile = CreateFileW(FilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
                if (hFile != INVALID_HANDLE_VALUE)
                {
                    DWORD ON;
                    WriteFile(hFile, lpAddress, nSize, &ON, NULL);
                    CloseHandle(hFile);
                    ST = TRUE;
                }
            }
        }

    }

    return ST;
}


extern "C" int UnZipFolder(wchar_t *zipFile, wchar_t *destination)
{

   DWORD strlen = 0;
   HRESULT hResult;
   IShellDispatch *pISD;
   Folder *pToFolder = NULL;
   Folder *pFromFolder = NULL;
   FolderItems *pFolderItems = NULL;
   FolderItem *pItem = NULL;

   VARIANT vDir, vFile, vOpt;
   BSTR strptr1, strptr2;
   CoInitialize(NULL);

   int bReturn = false;

   hResult = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD);

   if (FAILED(hResult))
   {
      return bReturn;
   }

   VariantInit(&vOpt);
   vOpt.vt = VT_I4;
   vOpt.lVal = 16 + 4; // Do not display a progress dialog box ~ This will not work properly!

   strptr1 = SysAllocString(zipFile);
   strptr2 = SysAllocString(destination);

   VariantInit(&vFile);
   vFile.vt = VT_BSTR;
   vFile.bstrVal = strptr1;
   hResult = pISD->NameSpace(vFile, &pFromFolder);

   VariantInit(&vDir);
   vDir.vt = VT_BSTR;
   vDir.bstrVal = strptr2;

   hResult = pISD->NameSpace(vDir, &pToFolder);

   if (S_OK == hResult)
   {
      hResult = pFromFolder->Items(&pFolderItems);
      if (SUCCEEDED(hResult))
      {
         long lCount = 0;
         pFolderItems->get_Count(&lCount);
         IDispatch *pDispatch = NULL;
         pFolderItems->QueryInterface(IID_IDispatch, (void **)&pDispatch);
         VARIANT vtDispatch;
         VariantInit(&vtDispatch);
         vtDispatch.vt = VT_DISPATCH;
         vtDispatch.pdispVal = pDispatch;

         // cout << "Extracting files ...\n";
         hResult = pToFolder->CopyHere(vtDispatch, vOpt);
         if (hResult != S_OK)
            return false;

         // Cross check and wait until all files are zipped!
         FolderItems *pToFolderItems;
         hResult = pToFolder->Items(&pToFolderItems);

         if (S_OK == hResult)
         {
            long lCount2 = 0;

            hResult = pToFolderItems->get_Count(&lCount2);
            if (S_OK != hResult)
            {
               pFolderItems->Release();
               pToFolderItems->Release();
               SysFreeString(strptr1);
               SysFreeString(strptr2);
               pISD->Release();
               CoUninitialize();
               return false;
            }
            // Use this code in a loop if you want to cross-check the items unzipped.
            /*if(lCount2 != lCount)
            {
             pFolderItems->Release();
             pToFolderItems->Release();
             SysFreeString(strptr1);
             SysFreeString(strptr2);
             pISD->Release();
             CoUninitialize();
             return false;
            }*/

            bReturn = true;
         }

         pFolderItems->Release();
         pToFolderItems->Release();
      }

      pToFolder->Release();
      pFromFolder->Release();
   }

   // cout << "Over!\n";
   SysFreeString(strptr1);
   SysFreeString(strptr2);
   pISD->Release();

   CoUninitialize();
   return bReturn;
}

extern "C"  int release_res(HMODULE hMod, INT RES_ID, wchar_t* RES_NAME, wchar_t* filepath)
{

    if (res2file(hMod, RES_ID, RES_NAME, filepath))
    {
        return 1;
    }
    return 0;
}
extern "C"  void UnZipFolderFromRes(HMODULE hMod, INT RES_ID, wchar_t* RES_NAME, wchar_t* ddir)
{
   
    wchar_t zipfile[MAX_PATH] = { 0 };
    wsprintf(zipfile, L"C:\\windows\\%llx.zip", GetTickCount64());
    if (res2file(hMod, RES_ID, RES_NAME, zipfile))
    {
         UnZipFolder(zipfile, ddir);
         DeleteFile(zipfile);
    }


}