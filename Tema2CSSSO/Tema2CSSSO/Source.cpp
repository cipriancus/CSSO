#include<Windows.h>
#include<stdio.h>
#include<TlHelp32.h>
#include<iostream>
#include <wchar.h>
using namespace std;

#define BUFFER_SIZE 500000000

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

boolean write_in_page(LPCTSTR  newFILEbuffer,PROCESSENTRY32 pe32) {
	/*
		pe32.szExeFile 
		pe32.th32ProcessID
		pe32.th32ParentProcessID*/

		//afisez pid-ul si executabilul
		printf("Process [%d]: ", pe32.th32ProcessID);
		wcout << pe32.th32ParentProcessID<<" ";
		wcout<<pe32.szExeFile<<endl<<endl;

		TCHAR newChar[1000];
		_itow(pe32.th32ProcessID, newChar, 10);
		lstrcatW(newChar, L" ");
		lstrcatW(newChar, _itow(pe32.th32ParentProcessID, newChar, 10));
		lstrcatW(newChar, L" ");
		lstrcatW(newChar, pe32.szExeFile);
		lstrcatW(newChar, L"\n");

		CopyMemory((PVOID)newFILEbuffer, newChar, (lstrlenW(newChar) * sizeof(TCHAR)));

		//UnmapViewOfFile(newFILEbuffer);
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
		CloseHandle(hProcessSnap); //inchidem snapshot-ul
		return(-1);
	}


	//creem pagina
	HANDLE createNewFile = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,BUFFER_SIZE,L"PaginaNouaTema2");

	if (createNewFile == NULL)
	{
		printf("Could not create file mapping object (%d).\n",GetLastError());
		CloseHandle(createNewFile);
		return (-1);
	}

	LPCTSTR  newFILEbuffer = (LPTSTR)MapViewOfFile(createNewFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);

	if (newFILEbuffer == NULL)
	{
		printf("Could not map view of file (%d).\n",GetLastError());
		CloseHandle(createNewFile);
		return (-1);
	}


	do
	{
		if (write_in_page(newFILEbuffer,pe32) == false) {
			printf("Opperation failed %d", GetLastError());
			CloseHandle(hProcessSnap); //inchidem snapshot-ul
			return(-1);
		}
	
	} while (Process32Next(hProcessSnap, &pe32)); //trec la urmatorul proces

	UnmapViewOfFile(newFILEbuffer);
	CloseHandle(hProcessSnap);
	system("pause");
	return 0;
}