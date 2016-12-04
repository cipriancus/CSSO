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

HANDLE hEvent;
HANDLE hEvent2;
HANDLE createNewFile = NULL;

void write_in_page(LPCTSTR  newFILEbuffer, const wchar_t* newChar) {
	memset((PVOID)newFILEbuffer, NULL, FILESIZE);
	CopyMemory((PVOID)newFILEbuffer, newChar, (lstrlenW(newChar)) * sizeof(wchar_t));
}

DWORD WINAPI proces1(LPVOID lpParam) {
	createNewFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, FILESIZE, L"PaginaNouaTema3");

	if (createNewFile == NULL)
	{
		printf("Could not create file mapping object (%d).\n", GetLastError());
		CloseHandle(createNewFile);
		ReleaseMutex(hEvent);
		cout << GetLastError();
		return (-1);
	}

	LPCTSTR  newFILEbuffer = (LPTSTR)MapViewOfFile(createNewFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if (newFILEbuffer == NULL)
	{
		printf("Could not map view of file (%d).\n", GetLastError());
		CloseHandle(createNewFile);
		ReleaseMutex(hEvent);
		UnmapViewOfFile(newFILEbuffer);
		return (-1);
	}

	SetEvent(hEvent2);

	for (int iterator = 0; iterator < 50; iterator++) {
		WaitForSingleObject(hEvent2, INFINITE);

		int a = rand() % 200;

		while (a == 0)
			a = rand() % 200;

		int b = rand() % 200;

		while (b == 0)
			b = rand() % 200;

		cout << a << " " << b << endl;

		wstring values = L"\n" + to_wstring(a) + L"\n" + to_wstring(b) + L"\n";

		write_in_page(newFILEbuffer, values.c_str());
		ResetEvent(hEvent2);
		SetEvent(hEvent);
	}
	return 1;
}

DWORD WINAPI proces2(LPVOID lpParam) {
main_for:
	for (int iterator = 0; iterator < 50; ) {
		WaitForSingleObject(hEvent, INFINITE);

			LPVOID  newFILEbuffer = MapViewOfFile(createNewFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

			if (newFILEbuffer == NULL)
			{
				printf("Could not map view of file (%d).\n", GetLastError());
				CloseHandle(createNewFile);
				ReleaseMutex(hEvent);
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
					cout << iterator << " " << a << " " << b << endl;
				}
				else {
					cout << "Corect la pasul ";
					cout << iterator << " " << a << " " << b << endl;
				}

				token = wcstok(NULL, L"\n", &buffer);//citesc in continuare
			}

			if (iterator + 1 == 200) {
				UnmapViewOfFile(newFILEbuffer);
				CloseHandle(createNewFile);
			}

			SetEvent(hEvent2);
			ResetEvent(hEvent);
			iterator++;
	}
	return 1;
}

int main() {
	//creare event
	hEvent = CreateEvent(NULL,true,false, NULL);
	hEvent2 = CreateEvent(NULL, true, false, NULL);

	if (hEvent == INVALID_HANDLE_VALUE || hEvent2 == INVALID_HANDLE_VALUE) {
		cout << "Error at creating handle";
			return (-1);
	}

	//crearea firului de executie care va scrie date
	HANDLE hThread1 = CreateThread(NULL, NULL, proces1, NULL, NULL, NULL);
	//creare firului de executie care va citi datele
	HANDLE hThread2 = CreateThread(NULL, NULL, proces2, NULL, NULL, NULL);

	if (hThread1 == INVALID_HANDLE_VALUE && hThread2 == INVALID_HANDLE_VALUE) {
		cout << "Error at creating threads";
		CloseHandle(hEvent);
		CloseHandle(hEvent2);
		return (-1);
	}

	//asteptam cele 2 fire de executie sa termine
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	CloseHandle(hEvent);
	CloseHandle(hEvent2);
	system("pause");
	return 0;
}

