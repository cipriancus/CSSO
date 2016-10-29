#include<Windows.h>
#include<stdio.h>
#include<TlHelp32.h>
#include<iostream>
#include <tchar.h>
#include <wchar.h>
#include <string.h>
#include<map>
#include<fstream>
using namespace std;

#define FILESIZE 1000000

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

boolean write_in_page(LPCTSTR  newFILEbuffer,const wchar_t* newChar) {
		CopyMemory((PVOID)newFILEbuffer, newChar, (lstrlenW(newChar))*sizeof(wchar_t));
		return true;
}


int main() {

	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	//cer un snapshot la procese
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//TH32CS_SNAPPROCESS lista proceselor

	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot failed.err = %d \n", GetLastError());
		CloseHandle(hProcessSnap);
		return(-1);
	}


	//initializez dwSize cu dimensiunea structurii.
	pe32.dwSize = sizeof(PROCESSENTRY32);


	//obtin informatii despre primul proces
	if (!Process32First(hProcessSnap, &pe32))
	{
		printf("Process32First failed. err = %d \n", GetLastError());
		CloseHandle(hProcessSnap); 
		return(-1);
	}

	/*
	dwMaximumSizeHigh
	[in] Specifies the high order 32 bits of the maximum size of the file-mapping object.
	If you intend to grow the file, specify the maximum file size so that the kernel can reserve the correct amount of memory.

	dwMaximumSizeLow
	[in] Specifies the low order 32 bits of the maximum size of the file-mapping object. If this parameter and
	dwMaximumSizeHigh are set to zero, the maximum size of the file-mapping object is equal to the current size of the file specified by hFile.
	If dwMaximumSizeLow is set to zero, the function returns an error that indicates an invalid parameter.*/

	//creem pagina
	HANDLE createNewFile = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0, FILESIZE,L"PaginaNouaTema2");

	if (createNewFile == NULL)
	{
		printf("Could not create file mapping object (%d).\n",GetLastError());
		CloseHandle(createNewFile);
		CloseHandle(hProcessSnap);
		cout << GetLastError();
		return (-1);
	}

	LPCTSTR  newFILEbuffer = (LPTSTR)MapViewOfFile(createNewFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if (newFILEbuffer == NULL)
	{
		printf("Could not map view of file (%d).\n",GetLastError());
		CloseHandle(createNewFile);
		CloseHandle(hProcessSnap);
		UnmapViewOfFile(newFILEbuffer);
		return (-1);
	}

	wstring string;
	do
	{
		TCHAR newChar[10000]=L"<";
		TCHAR newPID[100];


		_itow(pe32.th32ProcessID, newPID, 10);
		lstrcatW(newChar, newPID);
		lstrcatW(newChar, L">");


		lstrcatW(newChar, L"<");
		_itow(pe32.th32ParentProcessID, newPID, 10);
		lstrcatW(newChar, newPID);
		lstrcatW(newChar, L">");


		lstrcatW(newChar, L"<");
		lstrcatW(newChar, pe32.szExeFile);
		lstrcatW(newChar, L">");

		lstrcatW(newChar, L"\n");
		
		string = string + wstring(newChar);
	} while (Process32Next(hProcessSnap, &pe32));

	//wcout << string.c_str();

	if (write_in_page(newFILEbuffer, string.c_str()) == false) {
		printf("Opperation failed %d", GetLastError());
		UnmapViewOfFile(newFILEbuffer);
		CloseHandle(createNewFile);
		CloseHandle(hProcessSnap); 
		return(-1);
	}

	cout << "Successful opperation" << endl;

	//CloseHandle(createNewFile);
	UnmapViewOfFile(newFILEbuffer);
	CloseHandle(hProcessSnap);
	system("pause");
	return 0;
}
