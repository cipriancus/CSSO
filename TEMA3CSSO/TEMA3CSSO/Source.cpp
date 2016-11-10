#include<Windows.h>
#include<stdio.h>
#include<TlHelp32.h>
#include<iostream>
#include <tchar.h>
#include <wchar.h>
#include <string>
#include<map>
#include<fstream>
#include<vector>
using namespace std;

#define FILESIZE 1000000

HANDLE hMutex;
HANDLE createNewFile=NULL;

void write_in_page(LPCTSTR  newFILEbuffer, const wchar_t* newChar) {
	CopyMemory((PVOID)newFILEbuffer, newChar, (lstrlenW(newChar)) * sizeof(wchar_t));
}

DWORD WINAPI proces1(LPVOID lpParam) {
	wstring values = L"\n";

	for (int iterator = 0; iterator < 200; iterator++) {
		WaitForSingleObject(hMutex, INFINITE);

		createNewFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, FILESIZE, L"PaginaNouaTema3");

		if (createNewFile == NULL)
		{
			printf("Could not create file mapping object (%d).\n", GetLastError());
			CloseHandle(createNewFile);
			ReleaseMutex(hMutex);
			cout << GetLastError();
			return (-1);
		}

		LPCTSTR  newFILEbuffer = (LPTSTR)MapViewOfFile(createNewFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (newFILEbuffer == NULL)
		{
			printf("Could not map view of file (%d).\n", GetLastError());
			CloseHandle(createNewFile);
			ReleaseMutex(hMutex);
			UnmapViewOfFile(newFILEbuffer);
			return (-1);
		}

		int a = rand();

		while (a == 0)
			a = rand();

		int b = 2 * a;

		values = values +to_wstring(a) + L"\n"+to_wstring(b)+L"\n";

		write_in_page(newFILEbuffer, values.c_str());
		ReleaseMutex(hMutex);
	}
	return 1;
}

DWORD WINAPI proces2(LPVOID lpParam) {
	main_for:
	for (int iterator = 0; iterator < 200; ) {
		WaitForSingleObject(hMutex, INFINITE);

		if (createNewFile != NULL) {

			LPVOID  newFILEbuffer = MapViewOfFile(createNewFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

			if (newFILEbuffer == NULL)
			{
				printf("Could not map view of file (%d).\n", GetLastError());
				CloseHandle(createNewFile);
				ReleaseMutex(hMutex);
				UnmapViewOfFile(newFILEbuffer);
				goto main_for;
			}


			TCHAR* p = (TCHAR*)newFILEbuffer;
			WCHAR *buffer;

			WCHAR *token = wcstok(p, L"\n", &buffer);//citesc primul \n
			while (token != NULL) {
				int a = _wtoi(token);
				token = wcstok(NULL, L"\n", &buffer);//citesc primul b
				int b = _wtoi(token);

				if (a * 2 != b) {
					//release mutex
					cout << "Incorect la pasul ";
					cout << iterator << endl;
					ReleaseMutex(hMutex);
					break;
				}
				else {
					cout << "Corect la pasul ";
					cout << iterator << endl;
					ReleaseMutex(hMutex);
					break;
				}

				token = wcstok(NULL, L"\n", &buffer);//citesc in continuare
			}

			if (iterator + 1 == 200) {
				UnmapViewOfFile(newFILEbuffer);
				CloseHandle(createNewFile);
			}

			ReleaseMutex(hMutex);
			iterator++;
		}
		else {
			ReleaseMutex(hMutex);
		}
	}
	return 1;
}

int main() {
	//creare mutex
	hMutex = CreateMutex(NULL, false, NULL);

	//crearea firului de executie care va scrie date
	HANDLE hThread1 = CreateThread(NULL, NULL, proces1, NULL, NULL, NULL);
	//creare firului de executie care va citi datele
	HANDLE hThread2 = CreateThread(NULL, NULL, proces2, NULL, NULL, NULL);

	//asteptam cele 2 fire de executie sa termine
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	//deinitializare obiect de tip mutex
	CloseHandle(hMutex);
	system("pause");
	return 0;
}

